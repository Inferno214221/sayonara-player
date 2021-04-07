/* GUI_TagEdit.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GUI_TagEdit.h"
#include "GUI_TagFromPath.h"
#include "TagLineEdit.h"
#include "GUI_CoverEdit.h"
#include "GUI_FailMessageBox.h"

#include "Gui/TagEdit/ui_GUI_TagEdit.h"

#include "Components/Tagging/Expression.h"
#include "Components/Tagging/Editor.h"

#include "Gui/Utils/Delegates/ComboBoxDelegate.h"
#include "Gui/Utils/Widgets/Completer.h"
#include "Gui/Utils/EventFilter.h"
#include "Gui/Utils/Style.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/FileUtils.h"
#include "Utils/Message/Message.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Language/Language.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include <QStringList>
#include <QPixmap>
#include <QFileInfo>

using namespace Tagging;

struct GUI_TagEdit::Private
{
	Tagging::Editor*	tagEditor=nullptr;
	GUI_TagFromPath*	uiTagFromPath=nullptr;
	GUI_CoverEdit*		uiCoverEdit=nullptr;
	int					currentIndex;
};

GUI_TagEdit::GUI_TagEdit(QWidget* parent) :
	Widget(parent)
{
	ui = new Ui::GUI_TagEdit();
	ui->setupUi(this);

	m = Pimpl::make<Private>();
	m->tagEditor = createEditor();
	m->uiTagFromPath = new GUI_TagFromPath(ui->tabFromPath);
	m->uiCoverEdit = new GUI_CoverEdit(this);

	ui->tabFromPath->layout()->addWidget(m->uiTagFromPath);
	ui->tabCover->layout()->addWidget(m->uiCoverEdit);

	ui->tabWidget->setCurrentIndex(0);
	ui->widgetRating->setMouseTrackable(false);

	connect(ui->btnNext, &QPushButton::clicked, this, &GUI_TagEdit::nextButtonClicked);
	connect(ui->btnPrev, &QPushButton::clicked, this, &GUI_TagEdit::prevButtonClicked);

	connect(ui->cbAlbumAll, &QCheckBox::toggled, ui->leAlbum, &QWidget::setDisabled);
	connect(ui->cbArtistAll, &QCheckBox::toggled, ui->leArtist, &QWidget::setDisabled);
	connect(ui->cbAlbumArtistAll, &QCheckBox::toggled, ui->leAlbumArtist, &QWidget::setDisabled);
	connect(ui->cbGenreAll, &QCheckBox::toggled, ui->leGenre, &QWidget::setDisabled);
	connect(ui->cbYearAll, &QCheckBox::toggled, ui->sbYear, &QWidget::setDisabled);
	connect(ui->cbDiscnumberAll, &QCheckBox::toggled, ui->sbDiscnumber, &QWidget::setDisabled);
	connect(ui->cbRatingAll, &QCheckBox::toggled, ui->widgetRating, &QWidget::setDisabled);
	connect(ui->cbCommentAll, &QCheckBox::toggled, ui->teComment, &QWidget::setDisabled);

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GUI_TagEdit::commit);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GUI_TagEdit::sigCancelled);
	connect(ui->btnUndo, &QPushButton::clicked, this, &GUI_TagEdit::undoClicked);
	connect(ui->btnUndoAll, &QPushButton::clicked, this, &GUI_TagEdit::undoAllClicked);
	connect(ui->btnLoadCompleteAlbum, &QPushButton::clicked, this, &GUI_TagEdit::loadEntireAlbum);

	connect(m->uiTagFromPath, &GUI_TagFromPath::sigApply, this, &GUI_TagEdit::applyTagFromPath);
	connect(m->uiTagFromPath, &GUI_TagFromPath::sigApplyAll, this, &GUI_TagEdit::applyAllTagFromPath);

	metadataChanged(m->tagEditor->metadata());
}

GUI_TagEdit::~GUI_TagEdit() = default;

Editor* GUI_TagEdit::createEditor()
{
	auto* editor = new Tagging::Editor();

	connect(editor, &Editor::sigProgress, this, &GUI_TagEdit::progressChanged);
	connect(editor, &Editor::sigMetadataReceived, this, &GUI_TagEdit::metadataChanged);
	connect(editor, &Editor::sigStarted, this, &GUI_TagEdit::commitStarted);
	connect(editor, &Editor::sigFinished, this, &GUI_TagEdit::commitFinished);

	return editor;
}

void GUI_TagEdit::runEditor(Editor* editor)
{
	auto* t = new QThread();

	editor->moveToThread(t);

	connect(editor, &Tagging::Editor::sigFinished, t, &QThread::quit);
	connect(editor, &Tagging::Editor::sigFinished, editor, [=](){
		editor->moveToThread(QApplication::instance()->thread());
	});

	connect(t, &QThread::started, editor, &Editor::commit);
	connect(t, &QThread::finished, t, &QObject::deleteLater);

	t->start();
}

static void setAll_text(QCheckBox* label, int n)
{
	QString text = Lang::get(Lang::All);
	text += QString(" (%1)").arg(n);

	label->setText(text);
}

void GUI_TagEdit::languageChanged()
{
	ui->retranslateUi(this);

	ui->labTitle->setText(Lang::get(Lang::Title));
	ui->labAlbum->setText(Lang::get(Lang::Album));
	ui->labArtist->setText(Lang::get(Lang::Artist));
	ui->labYear->setText(Lang::get(Lang::Year));
	ui->labGenres->setText(Lang::get(Lang::Genres));
	ui->labRatingText->setText(Lang::get(Lang::Rating));
	ui->labTrackNumber->setText(Lang::get(Lang::TrackNo).toFirstUpper());
	ui->labComment->setText(Lang::get(Lang::Comment));
	ui->btnLoadCompleteAlbum->setText(tr("Load complete album"));

	setAll_text(ui->cbAlbumAll, m->tagEditor->count());
	setAll_text(ui->cbArtistAll, m->tagEditor->count());
	setAll_text(ui->cbAlbumArtistAll, m->tagEditor->count());
	setAll_text(ui->cbGenreAll, m->tagEditor->count());
	setAll_text(ui->cbYearAll, m->tagEditor->count());
	setAll_text(ui->cbDiscnumberAll, m->tagEditor->count());
	setAll_text(ui->cbRatingAll, m->tagEditor->count());
	setAll_text(ui->cbCommentAll, m->tagEditor->count());

	ui->btnUndo->setText(Lang::get(Lang::Undo));

	ui->tabWidget->setTabText(0, tr("Metadata"));
	ui->tabWidget->setTabText(1, tr("Tags from path"));
	ui->tabWidget->setTabText(2, Lang::get(Lang::Covers));
}

int GUI_TagEdit::count() const
{
	return m->tagEditor->count();
}

Editor*GUI_TagEdit::editor() const
{
	return m->tagEditor;
}

QAbstractButton* GUI_TagEdit::saveButton()
{
	return ui->buttonBox->button(QDialogButtonBox::StandardButton::Save);
}

QAbstractButton* GUI_TagEdit::closeButton()
{
	return ui->buttonBox->button(QDialogButtonBox::StandardButton::Close);
}


void GUI_TagEdit::setMetadata(const MetaDataList& v_md)
{
	m->tagEditor->setMetadata(v_md);
}

void GUI_TagEdit::metadataChanged(const MetaDataList& md)
{
	Q_UNUSED(md)

	reset();
	languageChanged();

	QStringList filepaths;
	const MetaDataList& v_md = m->tagEditor->metadata();
	for(const MetaData& md : v_md)
	{
		filepaths << md.filepath();
	}

	ui->btnLoadCompleteAlbum->setVisible(m->tagEditor->canLoadEntireAlbum());
	ui->btnLoadCompleteAlbum->setEnabled(true);

	saveButton()->setEnabled(true);
	ui->btnUndo->setEnabled(true);
	ui->btnUndoAll->setEnabled(true);

	setCurrentIndex(0);
	refreshCurrentTrack();
}

void GUI_TagEdit::applyTagFromPath()
{
	m->uiTagFromPath->clearInvalidFilepaths();

	bool success = m->tagEditor->applyRegularExpression(m->uiTagFromPath->getRegexString(), m->currentIndex);
	if(success){
		ui->tabWidget->setCurrentIndex(0);
	}

	refreshCurrentTrack();
}

void GUI_TagEdit::applyAllTagFromPath()
{
	m->uiTagFromPath->clearInvalidFilepaths();

	int count = m->tagEditor->count();
	QString regex = m->uiTagFromPath->getRegexString();
	QStringList invalidFilepaths;

	for(int i=0; i<count; i++)
	{
		bool success = m->tagEditor->applyRegularExpression(regex, i);
		if(!success)
		{
			QString invalidFilepath = m->tagEditor->metadata(i).filepath();
			invalidFilepaths << invalidFilepath;
			m->uiTagFromPath->addInvalidFilepath(invalidFilepath);
		}
	}

	if(invalidFilepaths.count() > 0)
	{
		Message::Answer answer = Message::Answer::Yes;

		QStringList err;
		err << tr("Cannot apply expression to %n track(s)", "", invalidFilepaths.count());
		err << "";
		err << tr("Ignore these tracks?");

		answer = Message::question_yn(err.join("<br>"));
		if(answer != Message::Answer::Yes)
		{
			m->tagEditor->undoAll();
		}

		else
		{
			ui->tabWidget->setCurrentIndex(0);
		}
	}

	else
	{
		ui->tabWidget->setCurrentIndex(0);
	}

	refreshCurrentTrack();
}

bool GUI_TagEdit::checkIndex(int idx) const
{
	return Util::between(idx, m->tagEditor->count());
}

void GUI_TagEdit::setCurrentIndex(int index)
{
	m->currentIndex = index;
	m->uiCoverEdit->setCurrentIndex(index);
}

void GUI_TagEdit::nextButtonClicked()
{
	writeChanges(m->currentIndex);

	setCurrentIndex(m->currentIndex + 1);

	refreshCurrentTrack();
}


void GUI_TagEdit::prevButtonClicked()
{
	writeChanges(m->currentIndex);

	setCurrentIndex(m->currentIndex - 1);

	refreshCurrentTrack();
}

void GUI_TagEdit::refreshCurrentTrack()
{
	int n_tracks = m->tagEditor->count();

	ui->btnNext->setEnabled(m->currentIndex >= 0 && m->currentIndex < n_tracks - 1);
	ui->btnPrev->setEnabled(m->currentIndex > 0 && m->currentIndex < n_tracks);

	if(!checkIndex(m->currentIndex)) {
		return;
	}

	MetaData md = m->tagEditor->metadata(m->currentIndex);

	{ // set filepath label
		QString filepath_link = Util::createLink
		(
			md.filepath(),
			Style::isDark(),
			true,
			Util::File::getParentDirectory(md.filepath())
		);

		ui->labFilepath->setText(filepath_link);
		m->uiTagFromPath->setFilepath(md.filepath());

		QFileInfo fi(md.filepath());
		ui->labReadOnly->setVisible(!fi.isWritable());
	}

	ui->leTitle->setText(md.title());

	if(!ui->cbAlbumAll->isChecked()){
		ui->leAlbum->setText(md.album());
	}

	if(!ui->cbArtistAll->isChecked()){
		ui->leArtist->setText(md.artist());
	}

	if(!ui->cbAlbumArtistAll->isChecked()){
		ui->leAlbumArtist->setText(md.albumArtist());
	}

	if(!ui->cbGenreAll->isChecked()){
		ui->leGenre->setText( md.genresToList().join(", ") );
	}

	if(!ui->cbYearAll->isChecked()){
		ui->sbYear->setValue(md.year());
	}

	if(!ui->cbDiscnumberAll->isChecked()){
		ui->sbDiscnumber->setValue(md.discnumber());
	}

	if(!ui->cbRatingAll->isChecked()){
		ui->widgetRating->setRating(md.rating());
	}

	if(!ui->cbCommentAll->isChecked()){
		ui->teComment->setPlainText(md.comment());
	}

	bool isCover_supported = m->tagEditor->isCoverSupported(m->currentIndex);
	ui->tabCover->setEnabled(isCover_supported);
	if(!isCover_supported){
		ui->tabWidget->setCurrentIndex(0);
	}

	m->uiCoverEdit->refreshCurrentTrack();

	ui->sbTrackNumber->setValue(md.trackNumber());
	ui->labTrackIndex->setText(
		Lang::get(Lang::Track).toFirstUpper().space() +
		QString::number(m->currentIndex+1 ) + "/" + QString::number( n_tracks )
	);
}

void GUI_TagEdit::reset()
{
	setCurrentIndex(-1);

	m->uiTagFromPath->reset();
	m->uiCoverEdit->reset();

	ui->tabWidget->tabBar()->setEnabled(true);
	ui->cbAlbumAll->setChecked(false);
	ui->cbArtistAll->setChecked(false);
	ui->cbAlbumArtistAll->setChecked(false);
	ui->cbGenreAll->setChecked(false);
	ui->cbDiscnumberAll->setChecked(false);
	ui->cbRatingAll->setChecked(false);
	ui->cbYearAll->setChecked(false);
	ui->cbCommentAll->setChecked(false);

	ui->labTrackIndex ->setText(Lang::get(Lang::Track) + " 0/0");
	ui->btnPrev->setEnabled(false);
	ui->btnNext->setEnabled(false);

	ui->leAlbum->clear();
	ui->leArtist->clear();
	ui->leAlbumArtist->clear();
	ui->leTitle->clear();
	ui->leGenre->clear();
	ui->teComment->clear();
	ui->sbYear->setValue(0);
	ui->sbDiscnumber->setValue(0);
	ui->widgetRating->setRating(Rating::Zero);
	ui->sbTrackNumber->setValue(0);
	ui->leAlbum->setEnabled(true);
	ui->leArtist->setEnabled(true);
	ui->leAlbumArtist->setEnabled(true);
	ui->leGenre->setEnabled(true);
	ui->sbYear->setEnabled(true);
	ui->sbDiscnumber->setEnabled(true);
	ui->widgetRating->setEnabled(true);

	ui->labFilepath->clear();
	ui->pbProgress->setVisible(false);

	ui->btnLoadCompleteAlbum->setVisible(false);

	initCompleter();
}

void GUI_TagEdit::initCompleter()
{
	AlbumList albums;
	ArtistList artists;
	QStringList albumstr, artiststr, genrestr;

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->libraryDatabase(-1, 0);

	lib_db->getAllAlbums(albums, true);
	lib_db->getAllArtists(artists, true);
	Util::Set<Genre> genres = lib_db->getAllGenres();

	for(const Album& album : albums){
		if(!album.name().isEmpty()){
			albumstr << album.name();
		}
	}

	for(const Artist& artist : artists){
		if(!artist.name().isEmpty()){
			artiststr << artist.name();
		}
	}

	for(const Genre& genre : genres){
		if(!genre.name().isEmpty()){
			genrestr << genre.name();
		}
	}

	if(ui->leAlbum->completer()){
		ui->leAlbum->completer()->deleteLater();
	}

	if(ui->leArtist->completer()){
		ui->leArtist->completer()->deleteLater();
	}

	if(ui->leAlbumArtist->completer()){
		ui->leAlbumArtist->completer()->deleteLater();
	}

	if(ui->leGenre->completer()){
		ui->leGenre->completer()->deleteLater();
	}

	auto* album_completer = new Gui::Completer(albumstr, ui->leAlbum);
	ui->leAlbum->setCompleter(album_completer);

	auto* albumArtist_completer = new Gui::Completer(artiststr, ui->leAlbumArtist);
	ui->leAlbumArtist->setCompleter(albumArtist_completer);

	auto* artist_completer = new Gui::Completer(artiststr, ui->leArtist);
	ui->leArtist->setCompleter(artist_completer);

	auto* genre_completer = new Gui::Completer(genrestr, ui->leGenre);
	ui->leGenre->setCompleter(genre_completer);
}

void GUI_TagEdit::undoClicked()
{
	m->tagEditor->undo(m->currentIndex);
	refreshCurrentTrack();
}

void GUI_TagEdit::undoAllClicked()
{
	m->tagEditor->undoAll();
	refreshCurrentTrack();
}

void GUI_TagEdit::writeChanges(int idx)
{
	if( !checkIndex(idx) ) {
		return;
	}

	MetaData md = m->tagEditor->metadata(idx);

	md.setTitle(ui->leTitle->text());
	md.setArtist(ui->leArtist->text());
	md.setAlbum(ui->leAlbum->text());
	md.setAlbumArtist(ui->leAlbumArtist->text());
	md.setGenres(ui->leGenre->text().split(", "));
	md.setComment(ui->teComment->toPlainText());

	md.setDiscnumber(Disc(ui->sbDiscnumber->value()));
	md.setYear(Year(ui->sbYear->value()));
	md.setTrackNumber(TrackNum(ui->sbTrackNumber->value()));
	md.setRating(ui->widgetRating->rating());

	QPixmap cover = m->uiCoverEdit->selectedCover(idx);

	m->tagEditor->updateTrack(idx, md);
	m->tagEditor->updateCover(idx, cover);
}

void GUI_TagEdit::commit()
{
	if(!saveButton()->isEnabled()){
		return;
	}

	writeChanges(m->currentIndex);

	for(int i=0; i<m->tagEditor->count(); i++)
	{
		if(i == m->currentIndex) {
			continue;
		}

		MetaData md =m->tagEditor->metadata(i);

		if( ui->cbAlbumAll->isChecked()){
			md.setAlbum(ui->leAlbum->text());
		}
		if( ui->cbArtistAll->isChecked()){
			md.setArtist(ui->leArtist->text());
		}
		if( ui->cbAlbumArtistAll->isChecked()){
			md.setAlbumArtist(ui->leAlbumArtist->text());
		}
		if( ui->cbGenreAll->isChecked())
		{
			QStringList genres = ui->leGenre->text().split(", ");
			md.setGenres(genres);
		}

		if( ui->cbDiscnumberAll->isChecked() ){
			md.setDiscnumber(Disc(ui->sbDiscnumber->value()));
		}

		if( ui->cbRatingAll->isChecked()){
			md.setRating(ui->widgetRating->rating());
		}

		if( ui->cbYearAll->isChecked()){
			md.setYear(Year(ui->sbYear->value()));
		}

		if( ui->cbCommentAll->isChecked() ){
			md.setComment(ui->teComment->toPlainText());
		}

		m->tagEditor->updateTrack(i, md);

		QPixmap cover = m->uiCoverEdit->selectedCover(i);
		m->tagEditor->updateCover(i, cover);
	}

	runEditor(m->tagEditor);
}

void GUI_TagEdit::commitStarted()
{
	saveButton()->setEnabled(false);
	ui->btnUndo->setEnabled(false);
	ui->btnUndoAll->setEnabled(false);
	ui->btnLoadCompleteAlbum->setEnabled(false);

	ui->tabWidget->tabBar()->setEnabled(false);

	ui->pbProgress->setVisible(true);
	ui->pbProgress->setMinimum(0);
	ui->pbProgress->setMaximum(100);
}

void GUI_TagEdit::progressChanged(int val)
{
	if(val >= 0) {
		ui->pbProgress->setValue(val);
	}

	else {
		ui->pbProgress->setMinimum(0);
		ui->pbProgress->setMaximum(0);
	}
}

void GUI_TagEdit::commitFinished()
{
	saveButton()->setEnabled(true);
	ui->btnLoadCompleteAlbum->setEnabled(m->tagEditor->canLoadEntireAlbum());
	ui->tabWidget->tabBar()->setEnabled(true);
	ui->pbProgress->setVisible(false);

	QMap<QString, Editor::FailReason> failed_files = m->tagEditor->failedFiles();
	if(!failed_files.isEmpty())
	{
		auto* fmb = new GUI_FailMessageBox(this);
		fmb->setFailedFiles(failed_files);
		fmb->setModal(true);

		connect(fmb, &GUI_FailMessageBox::sigClosed, fmb, &QObject::deleteLater);
		fmb->show();
	}

	metadataChanged(m->tagEditor->metadata());
}

void GUI_TagEdit::showCloseButton(bool show)
{
	closeButton()->setVisible(show);
}

void GUI_TagEdit::showDefaultTab()
{
	ui->tabWidget->setCurrentIndex(0);
}

void GUI_TagEdit::showCoverTab()
{
	ui->tabWidget->setCurrentIndex(2);
}

void GUI_TagEdit::loadEntireAlbum()
{
	if(m->tagEditor->hasChanges())
	{
		Message::Answer answer =
		Message::question(tr("All changes will be lost") + ".\n" + Lang::get(Lang::Continue).question(), "GUI_TagEdit", Message::QuestionType::YesNo);

		if(answer == Message::Answer::No){
			return;
		}
	}

	m->tagEditor->loadEntireAlbum();
}

void GUI_TagEdit::showEvent(QShowEvent* e)
{
	Widget::showEvent(e);

	refreshCurrentTrack();

	ui->leTitle->setFocus();
	ui->widgetRating->setMaximumHeight(fontMetrics().height() * 2);
}
