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
	m->uiTagFromPath = new GUI_TagFromPath(ui->tab_from_path);
	m->uiCoverEdit = new GUI_CoverEdit(this);

	ui->tab_from_path->layout()->addWidget(m->uiTagFromPath);
	ui->tab_cover->layout()->addWidget(m->uiCoverEdit);

	ui->tab_widget->setCurrentIndex(0);
	ui->widget_rating->setMouseTrackable(false);

	connect(ui->btn_next, &QPushButton::clicked, this, &GUI_TagEdit::nextButtonClicked);
	connect(ui->btn_prev, &QPushButton::clicked, this, &GUI_TagEdit::prevButtonClicked);

	connect(ui->cb_album_all, &QCheckBox::toggled, ui->le_album, &QWidget::setDisabled);
	connect(ui->cb_artist_all, &QCheckBox::toggled, ui->le_artist, &QWidget::setDisabled);
	connect(ui->cb_album_artist_all, &QCheckBox::toggled, ui->le_album_artist, &QWidget::setDisabled);
	connect(ui->cb_genre_all, &QCheckBox::toggled, ui->le_genre, &QWidget::setDisabled);
	connect(ui->cb_year_all, &QCheckBox::toggled, ui->sb_year, &QWidget::setDisabled);
	connect(ui->cb_discnumber_all, &QCheckBox::toggled, ui->sb_discnumber, &QWidget::setDisabled);
	connect(ui->cb_rating_all, &QCheckBox::toggled, ui->widget_rating, &QWidget::setDisabled);
	connect(ui->cb_comment_all, &QCheckBox::toggled, ui->te_comment, &QWidget::setDisabled);

	connect(ui->btn_save, &QPushButton::clicked, this, &GUI_TagEdit::commit);
	connect(ui->btn_undo, &QPushButton::clicked, this, &GUI_TagEdit::undoClicked);
	connect(ui->btn_undo_all, &QPushButton::clicked, this, &GUI_TagEdit::undoAllClicked);
	connect(ui->btn_close, &QPushButton::clicked, this, &GUI_TagEdit::sigCancelled);

	connect(ui->btn_load_entire_album, &QPushButton::clicked, this, &GUI_TagEdit::loadEntireAlbum);

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

static void set_all_text(QCheckBox* label, int n)
{
	QString text = Lang::get(Lang::All);
	text += QString(" (%1)").arg(n);

	label->setText(text);
}

void GUI_TagEdit::languageChanged()
{
	ui->retranslateUi(this);

	ui->lab_track_title->setText(Lang::get(Lang::Title));
	ui->lab_album->setText(Lang::get(Lang::Album));
	ui->lab_artist->setText(Lang::get(Lang::Artist));
	ui->lab_year->setText(Lang::get(Lang::Year));
	ui->lab_genres->setText(Lang::get(Lang::Genres));
	ui->lab_rating_descr->setText(Lang::get(Lang::Rating));
	ui->lab_track_num->setText(Lang::get(Lang::TrackNo).toFirstUpper());
	ui->lab_comment->setText(Lang::get(Lang::Comment));
	ui->btn_load_entire_album->setText(tr("Load complete album"));

	set_all_text(ui->cb_album_all, m->tagEditor->count());
	set_all_text(ui->cb_artist_all, m->tagEditor->count());
	set_all_text(ui->cb_album_artist_all, m->tagEditor->count());
	set_all_text(ui->cb_genre_all, m->tagEditor->count());
	set_all_text(ui->cb_year_all, m->tagEditor->count());
	set_all_text(ui->cb_discnumber_all, m->tagEditor->count());
	set_all_text(ui->cb_rating_all, m->tagEditor->count());
	set_all_text(ui->cb_comment_all, m->tagEditor->count());

	ui->btn_undo->setText(Lang::get(Lang::Undo));
	ui->btn_close->setText(Lang::get(Lang::Close));
	ui->btn_save->setText(Lang::get(Lang::Save));

	ui->tab_widget->setTabText(0, tr("Metadata"));
	ui->tab_widget->setTabText(1, tr("Tags from path"));
	ui->tab_widget->setTabText(2, Lang::get(Lang::Covers));
}

int GUI_TagEdit::count() const
{
	return m->tagEditor->count();
}

Editor*GUI_TagEdit::editor() const
{
	return m->tagEditor;
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

	ui->btn_load_entire_album->setVisible(m->tagEditor->canLoadEntireAlbum());
	ui->btn_load_entire_album->setEnabled(true);

	ui->btn_save->setEnabled(true);
	ui->btn_undo->setEnabled(true);
	ui->btn_undo_all->setEnabled(true);


	setCurrentIndex(0);
	refreshCurrentTrack();
}

void GUI_TagEdit::applyTagFromPath()
{
	m->uiTagFromPath->clearInvalidFilepaths();

	bool success = m->tagEditor->applyRegularExpression(m->uiTagFromPath->getRegexString(), m->currentIndex);
	if(success){
		ui->tab_widget->setCurrentIndex(0);
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
			QString invalid_filepath = m->tagEditor->metadata(i).filepath();
			invalidFilepaths << invalid_filepath;
			m->uiTagFromPath->addInvalidFilepath(invalid_filepath);
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
			ui->tab_widget->setCurrentIndex(0);
		}
	}

	else
	{
		ui->tab_widget->setCurrentIndex(0);
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

	ui->btn_next->setEnabled(m->currentIndex >= 0 && m->currentIndex < n_tracks - 1);
	ui->btn_prev->setEnabled(m->currentIndex > 0 && m->currentIndex < n_tracks);

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

		ui->lab_filepath->setText(filepath_link);
		m->uiTagFromPath->setFilepath(md.filepath());

		QFileInfo fi(md.filepath());
		ui->lab_read_only->setVisible(!fi.isWritable());
	}

	ui->le_title->setText(md.title());

	if(!ui->cb_album_all->isChecked()){
		ui->le_album->setText(md.album());
	}

	if(!ui->cb_artist_all->isChecked()){
		ui->le_artist->setText(md.artist());
	}

	if(!ui->cb_album_artist_all->isChecked()){
		ui->le_album_artist->setText(md.albumArtist());
	}

	if(!ui->cb_genre_all->isChecked()){
		ui->le_genre->setText( md.genresToList().join(", ") );
	}

	if(!ui->cb_year_all->isChecked()){
		ui->sb_year->setValue(md.year());
	}

	if(!ui->cb_discnumber_all->isChecked()){
		ui->sb_discnumber->setValue(md.discnumber());
	}

	if(!ui->cb_rating_all->isChecked()){
		ui->widget_rating->setRating(md.rating());
	}

	if(!ui->cb_comment_all->isChecked()){
		ui->te_comment->setPlainText(md.comment());
	}

	bool is_cover_supported = m->tagEditor->isCoverSupported(m->currentIndex);
	ui->tab_cover->setEnabled(is_cover_supported);
	if(!is_cover_supported){
		ui->tab_widget->setCurrentIndex(0);
	}

	m->uiCoverEdit->refreshCurrentTrack();

	ui->sb_track_num->setValue(md.trackNumber());
	ui->lab_track_index->setText(
		Lang::get(Lang::Track).toFirstUpper().space() +
		QString::number(m->currentIndex+1 ) + "/" + QString::number( n_tracks )
	);
}

void GUI_TagEdit::reset()
{
	setCurrentIndex(-1);

	m->uiTagFromPath->reset();
	m->uiCoverEdit->reset();

	ui->tab_widget->tabBar()->setEnabled(true);
	ui->cb_album_all->setChecked(false);
	ui->cb_artist_all->setChecked(false);
	ui->cb_album_artist_all->setChecked(false);
	ui->cb_genre_all->setChecked(false);
	ui->cb_discnumber_all->setChecked(false);
	ui->cb_rating_all->setChecked(false);
	ui->cb_year_all->setChecked(false);
	ui->cb_comment_all->setChecked(false);

	ui->lab_track_index ->setText(Lang::get(Lang::Track) + " 0/0");
	ui->btn_prev->setEnabled(false);
	ui->btn_next->setEnabled(false);

	ui->le_album->clear();
	ui->le_artist->clear();
	ui->le_album_artist->clear();
	ui->le_title->clear();
	ui->le_genre->clear();
	ui->te_comment->clear();
	ui->sb_year->setValue(0);
	ui->sb_discnumber->setValue(0);
	ui->widget_rating->setRating(Rating::Zero);
	ui->sb_track_num->setValue(0);
	ui->le_album->setEnabled(true);
	ui->le_artist->setEnabled(true);
	ui->le_album_artist->setEnabled(true);
	ui->le_genre->setEnabled(true);
	ui->sb_year->setEnabled(true);
	ui->sb_discnumber->setEnabled(true);
	ui->widget_rating->setEnabled(true);

	ui->lab_filepath->clear();
	ui->pb_progress->setVisible(false);

	ui->btn_load_entire_album->setVisible(false);

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

	if(ui->le_album->completer()){
		ui->le_album->completer()->deleteLater();
	}

	if(ui->le_artist->completer()){
		ui->le_artist->completer()->deleteLater();
	}

	if(ui->le_album_artist->completer()){
		ui->le_album_artist->completer()->deleteLater();
	}

	if(ui->le_genre->completer()){
		ui->le_genre->completer()->deleteLater();
	}

	auto* album_completer = new Gui::Completer(albumstr, ui->le_album);
	ui->le_album->setCompleter(album_completer);

	auto* album_artist_completer = new Gui::Completer(artiststr, ui->le_album_artist);
	ui->le_album_artist->setCompleter(album_artist_completer);

	auto* artist_completer = new Gui::Completer(artiststr, ui->le_artist);
	ui->le_artist->setCompleter(artist_completer);

	auto* genre_completer = new Gui::Completer(genrestr, ui->le_genre);
	ui->le_genre->setCompleter(genre_completer);
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

	md.setTitle(ui->le_title->text());
	md.setArtist(ui->le_artist->text());
	md.setAlbum(ui->le_album->text());
	md.setAlbumArtist(ui->le_album_artist->text());
	md.setGenres(ui->le_genre->text().split(", "));
	md.setComment(ui->te_comment->toPlainText());

	md.setDiscnumber(Disc(ui->sb_discnumber->value()));
	md.setYear(Year(ui->sb_year->value()));
	md.setTrackNumber(TrackNum(ui->sb_track_num->value()));
	md.setRating(ui->widget_rating->rating());

	QPixmap cover = m->uiCoverEdit->selectedCover(idx);

	m->tagEditor->updateTrack(idx, md);
	m->tagEditor->updateCover(idx, cover);
}

void GUI_TagEdit::commit()
{
	if(!ui->btn_save->isEnabled()){
		return;
	}

	writeChanges(m->currentIndex);

	for(int i=0; i<m->tagEditor->count(); i++)
	{
		if(i == m->currentIndex) {
			continue;
		}

		MetaData md =m->tagEditor->metadata(i);

		if( ui->cb_album_all->isChecked()){
			md.setAlbum(ui->le_album->text());
		}
		if( ui->cb_artist_all->isChecked()){
			md.setArtist(ui->le_artist->text());
		}
		if( ui->cb_album_artist_all->isChecked()){
			md.setAlbumArtist(ui->le_album_artist->text());
		}
		if( ui->cb_genre_all->isChecked())
		{
			QStringList genres = ui->le_genre->text().split(", ");
			md.setGenres(genres);
		}

		if( ui->cb_discnumber_all->isChecked() ){
			md.setDiscnumber(Disc(ui->sb_discnumber->value()));
		}

		if( ui->cb_rating_all->isChecked()){
			md.setRating(ui->widget_rating->rating());
		}

		if( ui->cb_year_all->isChecked()){
			md.setYear(Year(ui->sb_year->value()));
		}

		if( ui->cb_comment_all->isChecked() ){
			md.setComment(ui->te_comment->toPlainText());
		}

		m->tagEditor->updateTrack(i, md);

		QPixmap cover = m->uiCoverEdit->selectedCover(i);
		m->tagEditor->updateCover(i, cover);
	}

	runEditor(m->tagEditor);
}

void GUI_TagEdit::commitStarted()
{
	ui->btn_save->setEnabled(false);
	ui->btn_undo->setEnabled(false);
	ui->btn_undo_all->setEnabled(false);
	ui->btn_load_entire_album->setEnabled(false);

	ui->tab_widget->tabBar()->setEnabled(false);

	ui->pb_progress->setVisible(true);
	ui->pb_progress->setMinimum(0);
	ui->pb_progress->setMaximum(100);
}

void GUI_TagEdit::progressChanged(int val)
{
	if(val >= 0) {
		ui->pb_progress->setValue(val);
	}

	else {
		ui->pb_progress->setMinimum(0);
		ui->pb_progress->setMaximum(0);
	}
}

void GUI_TagEdit::commitFinished()
{
	ui->btn_save->setEnabled(true);
	ui->btn_load_entire_album->setEnabled(m->tagEditor->canLoadEntireAlbum());
	ui->tab_widget->tabBar()->setEnabled(true);
	ui->pb_progress->setVisible(false);

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
	ui->btn_close->setVisible(show);
}

void GUI_TagEdit::showCoverTab()
{
	ui->tab_widget->setCurrentIndex(2);
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

void GUI_TagEdit::showEvent(QShowEvent *e)
{
	Widget::showEvent(e);
	refreshCurrentTrack();

	ui->le_title->setFocus();
}
