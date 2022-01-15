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
#include "GUI_CoverEdit.h"
#include "GUI_FailMessageBox.h"

#include "Gui/TagEdit/ui_GUI_TagEdit.h"

#include "Components/Tagging/Editor.h"

#include "Gui/Utils/Widgets/Completer.h"
#include "Gui/Utils/Style.h"

#include "Utils/Algorithm.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/FileUtils.h"
#include "Utils/Message/Message.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Language/Language.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include <QFileInfo>
#include <QTabBar>

using namespace Tagging;

namespace
{
	struct WidgetTuple
	{
		QLabel* label;
		QWidget* widget;
		QCheckBox* checkbox;
		Lang::Term term;
	};

	void setCheckboxText(QCheckBox* checkbox, int n)
	{
		if(checkbox)
		{
			const auto text = QString("%1 (%2)")
				.arg(Lang::get(Lang::All))
				.arg(n);

			checkbox->setText(text);
		}
	}

	template<typename Container>
	QStringList extractNames(const Container& container)
	{
		QStringList names;
		for(const auto& item : container)
		{
			if(!item.name().isEmpty())
			{
				names << item.name();
			}
		}

		return names;
	}

	Message::Answer showInvalidFilepathsMessage(int count, QObject* parent)
	{
		const auto message = QString("%1<br><br>%2")
			.arg(parent->tr("Cannot apply expression to %n track(s)", "", count))
			.arg(parent->tr("Ignore these tracks?"));

		return Message::question_yn(message);
	}

	Message::Answer showAllChangesLostMessage(QObject* parent)
	{
		const auto question = QString("%1.\n%2?")
			.arg(parent->tr("All changes will be lost"))
			.arg(Lang::get(Lang::Continue));

		return Message::question(question, "GUI_TagEdit", Message::QuestionType::YesNo);
	}

	void showFailedCommitMessageBox(const QMap<QString, Tagging::Editor::FailReason>& failedFiles, QWidget* parent)
	{
		auto* failMessageBox = new GUI_FailMessageBox(parent);
		failMessageBox->setFailedFiles(failedFiles);
		failMessageBox->setModal(true);

		parent->connect(failMessageBox, &GUI_FailMessageBox::sigClosed, failMessageBox, &QObject::deleteLater);
		failMessageBox->show();
	}

	QStringList applyRegularExpression(int count, const QString& regex, Tagging::Editor* tagEditor)
	{
		QStringList invalidFilepaths;

		for(auto i = 0; i < count; i++)
		{
			const auto success = tagEditor->applyRegularExpression(regex, i);
			if(!success)
			{
				const auto invalidFilepath = tagEditor->metadata(i).filepath();
				invalidFilepaths << invalidFilepath;
			}
		}

		return invalidFilepaths;
	}

	QCompleter* addCompleter(const QStringList& dataList, QLineEdit* lineEdit)
	{
		if(lineEdit->completer())
		{
			lineEdit->completer()->deleteLater();
		}

		auto* completer = new Gui::Completer(dataList, lineEdit);
		lineEdit->setCompleter(completer);

		return completer;
	}

	void initCompleter(Ui::GUI_TagEdit* ui)
	{
		auto* db = DB::Connector::instance();
		auto* libraryDatabase = db->libraryDatabase(-1, 0);

		AlbumList albums;
		ArtistList artists;
		libraryDatabase->getAllAlbums(albums, true);
		libraryDatabase->getAllArtists(artists, true);
		const auto genres = libraryDatabase->getAllGenres();

		const auto albumNames = extractNames(albums);
		const auto artistNames = extractNames(artists);
		const auto genreNames = extractNames(genres);

		addCompleter(albumNames, ui->leAlbum);
		addCompleter(artistNames, ui->leAlbumArtist);
		addCompleter(artistNames, ui->leArtist);
		addCompleter(genreNames, ui->leGenre);
	}

	QPushButton* saveButton(Ui::GUI_TagEdit* ui)
	{
		return ui->buttonBox->button(QDialogButtonBox::StandardButton::Save);
	}
}

struct GUI_TagEdit::Private
{
	Tagging::Editor* tagEditor;
	GUI_TagFromPath* uiTagFromPath;
	GUI_CoverEdit* uiCoverEdit;
	QList<WidgetTuple> widgetTuples;
	int currentIndex;

	Private(GUI_TagEdit* parent, Ui::GUI_TagEdit* ui) :
		tagEditor {new Tagging::Editor()},
		uiTagFromPath {new GUI_TagFromPath(tagEditor, ui->tabFromPath)},
		uiCoverEdit {new GUI_CoverEdit(tagEditor, parent)},
		currentIndex {-1}
	{
		ui->tabFromPath->layout()->addWidget(uiTagFromPath);
		ui->tabCover->layout()->addWidget(uiCoverEdit);
		ui->tabWidget->setCurrentIndex(0);
		ui->widgetRating->setMouseTrackable(false);

		widgetTuples = QList<WidgetTuple>
			{{ui->labTitle,       ui->leTitle,       nullptr,              Lang::Title},
			 {ui->labTrackNumber, ui->sbTrackNumber, nullptr,              Lang::TrackNo},
			 {ui->labAlbum,       ui->leAlbum,       ui->cbAlbumAll,       Lang::Album},
			 {ui->labArtist,      ui->leArtist,      ui->cbArtistAll,      Lang::Artist},
			 {ui->labAlbumArtist, ui->leAlbumArtist, ui->cbAlbumArtistAll, Lang::AlbumArtist},
			 {ui->labGenres,      ui->leGenre,       ui->cbGenreAll,       Lang::Genre},
			 {ui->labYear,        ui->sbYear,        ui->cbYearAll,        Lang::Year},
			 {ui->labDiscnumber,  ui->sbDiscnumber,  ui->cbDiscnumberAll,  Lang::Disc},
			 {ui->labRatingText,  ui->widgetRating,  ui->cbRatingAll,      Lang::Rating},
			 {ui->labComment,     ui->teComment,     ui->cbCommentAll,     Lang::Comment}};
	}
};

GUI_TagEdit::GUI_TagEdit(QWidget* parent) :
	Widget(parent)
{
	ui = new Ui::GUI_TagEdit();
	ui->setupUi(this);

	m = Pimpl::make<Private>(this, ui);

	connect(m->tagEditor, &Editor::sigProgress, this, &GUI_TagEdit::progressChanged);
	connect(m->tagEditor, &Editor::sigMetadataReceived, this, &GUI_TagEdit::metadataChanged);
	connect(m->tagEditor, &Editor::sigStarted, this, &GUI_TagEdit::commitStarted);
	connect(m->tagEditor, &Editor::sigFinished, this, &GUI_TagEdit::commitFinished);

	for(const auto& widgetTuple : m->widgetTuples)
	{
		if(widgetTuple.checkbox)
		{
			connect(widgetTuple.checkbox, &QCheckBox::toggled, widgetTuple.widget, &QWidget::setDisabled);
		}
	}

	connect(ui->btnNext, &QPushButton::clicked, this, &GUI_TagEdit::nextButtonClicked);
	connect(ui->btnPrev, &QPushButton::clicked, this, &GUI_TagEdit::prevButtonClicked);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GUI_TagEdit::commit);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GUI_TagEdit::sigCancelled);
	connect(ui->btnUndo, &QPushButton::clicked, this, &GUI_TagEdit::undoClicked);
	connect(ui->btnUndoAll, &QPushButton::clicked, this, &GUI_TagEdit::undoAllClicked);
	connect(ui->btnLoadCompleteAlbum, &QPushButton::clicked, this, &GUI_TagEdit::loadEntireAlbumClicked);

	connect(m->uiTagFromPath, &GUI_TagFromPath::sigApply, this, &GUI_TagEdit::applyTagFromPathTriggered);
	connect(m->uiTagFromPath, &GUI_TagFromPath::sigApplyAll, this, &GUI_TagEdit::applyAllTagFromPathTriggered);

	metadataChanged(m->tagEditor->metadata());
}

GUI_TagEdit::~GUI_TagEdit() = default;

void GUI_TagEdit::runEditor(Editor* editor)
{
	auto* t = new QThread();
	editor->moveToThread(t);

	connect(editor, &Tagging::Editor::sigFinished, t, &QThread::quit);
	connect(editor, &Tagging::Editor::sigFinished, editor, [=]() {
		editor->moveToThread(QApplication::instance()->thread());
	});

	connect(t, &QThread::started, editor, &Editor::commit);
	connect(t, &QThread::finished, t, &QObject::deleteLater);

	t->start();
}

void GUI_TagEdit::languageChanged()
{
	for(const auto& widgetTuple : m->widgetTuples)
	{
		widgetTuple.label->setText(Lang::get(widgetTuple.term));
		setCheckboxText(widgetTuple.checkbox, m->tagEditor->count());
	}

	ui->btnLoadCompleteAlbum->setText(tr("Load complete album"));
	ui->btnUndo->setText(Lang::get(Lang::Undo));
	ui->tabWidget->setTabText(0, tr("Metadata"));
	ui->tabWidget->setTabText(1, tr("Tags from path"));
	ui->tabWidget->setTabText(2, Lang::get(Lang::Covers));

	ui->retranslateUi(this);
}

void GUI_TagEdit::setMetadata(const MetaDataList& tracks)
{
	m->tagEditor->setMetadata(tracks);
}

void GUI_TagEdit::metadataChanged([[maybe_unused]] const MetaDataList& changedTracks)
{
	reset();

	for(const auto& widgetTuple : m->widgetTuples)
	{
		setCheckboxText(widgetTuple.checkbox, m->tagEditor->count());
	}

	ui->btnLoadCompleteAlbum->setVisible(m->tagEditor->canLoadEntireAlbum());
	ui->btnLoadCompleteAlbum->setEnabled(true);

	saveButton(ui)->setEnabled(true);
	ui->btnUndo->setEnabled(true);
	ui->btnUndoAll->setEnabled(true);

	setCurrentIndex(0);
	refreshCurrentTrack();
}

void GUI_TagEdit::applyTagFromPathTriggered()
{
	const auto success = m->tagEditor->applyRegularExpression(m->uiTagFromPath->getRegexString(), m->currentIndex);
	if(success)
	{
		ui->tabWidget->setCurrentIndex(0);
	}

	refreshCurrentTrack();
}

void GUI_TagEdit::applyAllTagFromPathTriggered()
{
	const auto count = m->tagEditor->count();
	const auto regex = m->uiTagFromPath->getRegexString();
	const auto invalidFilepaths = applyRegularExpression(count, regex, m->tagEditor);
	auto isValid = invalidFilepaths.isEmpty();

	if(!invalidFilepaths.isEmpty())
	{
		for(const auto& invalidFilepath : invalidFilepaths)
		{
			m->uiTagFromPath->addInvalidFilepath(invalidFilepath);
		}

		const auto answer = showInvalidFilepathsMessage(invalidFilepaths.count(), this);
		isValid = (answer == Message::Answer::Yes);
		if(!isValid)
		{
			m->tagEditor->undoAll();
		}
	}

	if(isValid)
	{
		ui->tabWidget->setCurrentIndex(0);
	}

	refreshCurrentTrack();
}

void GUI_TagEdit::setCurrentIndex(int index)
{
	m->currentIndex = index;
	m->uiCoverEdit->setCurrentIndex(index);
	m->uiTagFromPath->setCurrentIndex(index);
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
	const auto trackCount = m->tagEditor->count();
	const auto isNotLast = (m->currentIndex >= 0) && (m->currentIndex < trackCount - 1);
	const auto isNotFirst = (m->currentIndex > 0) && (m->currentIndex < trackCount);

	ui->btnNext->setEnabled(isNotLast);
	ui->btnPrev->setEnabled(isNotFirst);

	if(!Util::between(m->currentIndex, m->tagEditor->count()))
	{
		return;
	}

	const auto track = m->tagEditor->metadata(m->currentIndex);

	{ // set filepath label
		const auto filepathLink =
			Util::createLink(track.filepath(), Style::isDark(), true, Util::File::getParentDirectory(track.filepath()));

		const auto fileInfo = QFileInfo(track.filepath());
		ui->labReadOnly->setVisible(!fileInfo.isWritable());
		ui->labFilepath->setText(filepathLink);
	}

	ui->leTitle->setText(track.title());

	if(!ui->cbAlbumAll->isChecked())
	{
		ui->leAlbum->setText(track.album());
	}

	if(!ui->cbArtistAll->isChecked())
	{
		ui->leArtist->setText(track.artist());
	}

	if(!ui->cbAlbumArtistAll->isChecked())
	{
		ui->leAlbumArtist->setText(track.albumArtist());
	}

	if(!ui->cbGenreAll->isChecked())
	{
		ui->leGenre->setText(track.genresToList().join(", "));
	}

	if(!ui->cbYearAll->isChecked())
	{
		ui->sbYear->setValue(track.year());
	}

	if(!ui->cbDiscnumberAll->isChecked())
	{
		ui->sbDiscnumber->setValue(track.discnumber());
	}

	if(!ui->cbRatingAll->isChecked())
	{
		ui->widgetRating->setRating(track.rating());
	}

	if(!ui->cbCommentAll->isChecked())
	{
		ui->teComment->setPlainText(track.comment());
	}

	const auto isCoverSupported = m->tagEditor->isCoverSupported(m->currentIndex);
	ui->tabCover->setEnabled(isCoverSupported);
	if(!isCoverSupported)
	{
		ui->tabWidget->setCurrentIndex(0);
	}

	m->uiCoverEdit->refreshCurrentTrack();
	m->uiTagFromPath->refreshCurrentTrack();

	ui->sbTrackNumber->setValue(track.trackNumber());

	const auto trackIndexText = QString("%1 %2/%3")
		.arg(Lang::get(Lang::Track).toFirstUpper())
		.arg(m->currentIndex + 1)
		.arg(trackCount);

	ui->labTrackIndex->setText(trackIndexText);
}

void GUI_TagEdit::reset()
{
	setCurrentIndex(-1);

	m->uiTagFromPath->reset();
	m->uiCoverEdit->reset();

	ui->tabWidget->tabBar()->setEnabled(true);

	for(const auto& widgetTuple : m->widgetTuples)
	{
		if(widgetTuple.checkbox)
		{
			widgetTuple.checkbox->setChecked(false);
		}

		widgetTuple.widget->setEnabled(true);
	}

	ui->leTitle->clear();
	ui->labTrackIndex->setText(Lang::get(Lang::Track) + " 0/0");
	ui->sbTrackNumber->setValue(0);
	ui->leAlbum->clear();
	ui->leArtist->clear();
	ui->leAlbumArtist->clear();
	ui->leGenre->clear();
	ui->sbYear->clear();
	ui->sbDiscnumber->clear();
	ui->teComment->clear();

	ui->btnPrev->setEnabled(false);
	ui->btnNext->setEnabled(false);

	ui->widgetRating->setRating(Rating::Zero);

	ui->labFilepath->clear();
	ui->pbProgress->setVisible(false);

	ui->btnLoadCompleteAlbum->setVisible(false);

	initCompleter(ui);
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

void GUI_TagEdit::writeChanges(int trackIndex)
{
	if(!Util::between(m->currentIndex, m->tagEditor->count()))
	{
		return;
	}

	auto track = m->tagEditor->metadata(trackIndex);

	track.setTitle(ui->leTitle->text());
	track.setArtist(ui->leArtist->text());
	track.setAlbum(ui->leAlbum->text());
	track.setAlbumArtist(ui->leAlbumArtist->text());
	track.setGenres(ui->leGenre->text().split(", "));
	track.setComment(ui->teComment->toPlainText());
	track.setDiscnumber(Disc(ui->sbDiscnumber->value()));
	track.setYear(Year(ui->sbYear->value()));
	track.setTrackNumber(TrackNum(ui->sbTrackNumber->value()));
	track.setRating(ui->widgetRating->rating());

	const auto cover = m->uiCoverEdit->selectedCover(trackIndex);

	m->tagEditor->updateTrack(trackIndex, track);
	m->tagEditor->updateCover(trackIndex, cover);
}

void GUI_TagEdit::commit()
{
	if(!saveButton(ui)->isEnabled())
	{
		return;
	}

	writeChanges(m->currentIndex);

	for(auto i = 0; i < m->tagEditor->count(); i++)
	{
		if(i == m->currentIndex)
		{
			continue;
		}

		auto track = m->tagEditor->metadata(i);

		if(ui->cbAlbumAll->isChecked())
		{
			track.setAlbum(ui->leAlbum->text());
		}
		if(ui->cbArtistAll->isChecked())
		{
			track.setArtist(ui->leArtist->text());
		}
		if(ui->cbAlbumArtistAll->isChecked())
		{
			track.setAlbumArtist(ui->leAlbumArtist->text());
		}
		if(ui->cbGenreAll->isChecked())
		{
			const auto genres = ui->leGenre->text().split(", ");
			track.setGenres(genres);
		}

		if(ui->cbDiscnumberAll->isChecked())
		{
			track.setDiscnumber(Disc(ui->sbDiscnumber->value()));
		}

		if(ui->cbRatingAll->isChecked())
		{
			track.setRating(ui->widgetRating->rating());
		}

		if(ui->cbYearAll->isChecked())
		{
			track.setYear(Year(ui->sbYear->value()));
		}

		if(ui->cbCommentAll->isChecked())
		{
			track.setComment(ui->teComment->toPlainText());
		}

		m->tagEditor->updateTrack(i, track);

		const auto cover = m->uiCoverEdit->selectedCover(i);
		m->tagEditor->updateCover(i, cover);
	}

	runEditor(m->tagEditor);
}

void GUI_TagEdit::commitStarted()
{
	saveButton(ui)->setEnabled(false);
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
	if(val >= 0)
	{
		ui->pbProgress->setValue(val);
	}

	else
	{
		ui->pbProgress->setMinimum(0);
		ui->pbProgress->setMaximum(0);
	}
}

void GUI_TagEdit::commitFinished()
{
	saveButton(ui)->setEnabled(true);
	ui->btnLoadCompleteAlbum->setEnabled(m->tagEditor->canLoadEntireAlbum());
	ui->tabWidget->tabBar()->setEnabled(true);
	ui->pbProgress->setVisible(false);

	const auto failedFiles = m->tagEditor->failedFiles();
	if(!failedFiles.isEmpty())
	{
		showFailedCommitMessageBox(failedFiles, this);
	}

	metadataChanged(m->tagEditor->metadata());
}

void GUI_TagEdit::showDefaultTab()
{
	ui->tabWidget->setCurrentIndex(0);
}

void GUI_TagEdit::showCoverTab()
{
	ui->tabWidget->setCurrentIndex(2);
}

void GUI_TagEdit::loadEntireAlbumClicked()
{
	if(m->tagEditor->hasChanges())
	{
		const auto answer = showAllChangesLostMessage(this);
		if(answer == Message::Answer::No)
		{
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
}
