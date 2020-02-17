/* GUI_LocalLibrary.cpp */

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


/*
 * GUI_LocalLibrary.cpp
 *
 *  Created on: Apr 24, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "GUI_LocalLibrary.h"
#include "Gui/Library/ui_GUI_LocalLibrary.h"

#include "Gui/Library/GUI_CoverView.h"
#include "Gui/Library/Utils/DirChooserDialog.h"
#include "Gui/Library/Utils/GUI_ReloadLibraryDialog.h"
#include "Gui/Library/Utils/GUI_LibraryInfoBox.h"
#include "Gui/Library/Utils/LocalLibraryMenu.h"

#include "Gui/ImportDialog/GUI_ImportDialog.h"
#include "Gui/Utils/Library/GUI_DeleteDialog.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Style.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QFileDialog>
#include <QStringList>

enum StatusWidgetIndex
{
	ReloadLibraryIndex=0,
	FileExtensionsIndex=1
};

enum GenreWidgetIndex
{
	GenreTree=0,
	NoGenres=1
};

enum AlbumViewIndex
{
	ArtistAlbumTableView=0,
	AlbumCoverView=1
};

enum ReloadWidgetIndex
{
	StandardView=0,
	ReloadView=1
};

using namespace Library;

struct GUI_LocalLibrary::Private
{
	LocalLibrary*			library = nullptr;
	LocalLibraryMenu*		library_menu = nullptr;

	Private(LibraryId id, GUI_LocalLibrary* parent)
	{
		library = Manager::instance()->libraryInstance(id);
		library_menu = new LocalLibraryMenu(library->name(), library->path(), parent);
	}
};


GUI_LocalLibrary::GUI_LocalLibrary(LibraryId id, QWidget* parent) :
	GUI_AbstractLibrary(Manager::instance()->libraryInstance(id), parent)
{
	m = Pimpl::make<Private>(id, this);

	setupParent(this, &ui);

	this->setFocusProxy(ui->le_search);

	connect(m->library, &LocalLibrary::sigReloadingLibrary, this, &GUI_LocalLibrary::progressChanged);
	connect(m->library, &LocalLibrary::sigReloadingLibraryFinished, this, &GUI_LocalLibrary::reloadFinished);
	connect(m->library, &LocalLibrary::sigReloadingLibraryFinished, ui->lv_genres, &GenreView::reloadGenres);
	connect(m->library, &LocalLibrary::sigAllTracksLoaded, this, &GUI_LocalLibrary::tracksLoaded);
	connect(m->library, &LocalLibrary::sigImportDialogRequested, this, &GUI_LocalLibrary::importDialogRequested);

	auto* manager = Manager::instance();
	connect(manager, &Manager::sigPathChanged, this, &GUI_LocalLibrary::pathChanged);
	connect(manager, &Manager::sigRenamed, this, &GUI_LocalLibrary::nameChanged);

	connect(ui->tv_albums, &AlbumView::sigDiscPressed, m->library, &LocalLibrary::changeCurrentDisc);
	connect(ui->lv_genres, &GenreView::sigSelectedChanged, this, &GUI_LocalLibrary::genreSelectionChanged);
	connect(ui->lv_genres, &GenreView::sigInvalidGenreSelected, this, &GUI_LocalLibrary::invalidGenreSelected);
	connect(ui->lv_genres, &GenreView::sigProgress, this, &GUI_LocalLibrary::progressChanged);

	connect(m->library_menu, &LocalLibraryMenu::sigPathChanged, m->library, &LocalLibrary::setLibraryPath);
	connect(m->library_menu, &LocalLibraryMenu::sigNameChanged, m->library, &LocalLibrary::setLibraryName);
	connect(m->library_menu, &LocalLibraryMenu::sigImportFile, this, &GUI_LocalLibrary::importFilesRequested);
	connect(m->library_menu, &LocalLibraryMenu::sigImportFolder, this, &GUI_LocalLibrary::importDirsRequested);
	connect(m->library_menu, &LocalLibraryMenu::sigInfo, this, &GUI_LocalLibrary::showInfoBox);
	connect(m->library_menu, &LocalLibraryMenu::sigReloadLibrary, this, &GUI_LocalLibrary::reloadLibraryRequested);

	connect(ui->btn_scanForFiles, &QPushButton::clicked, this, &GUI_LocalLibrary::reloadLibraryDeepRequested);
	connect(ui->btn_importDirectories, &QPushButton::clicked, this, &GUI_LocalLibrary::importDirsRequested);

	connect(ui->splitter_artistAlbum, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitterArtistMoved);
	connect(ui->splitter_tracks, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitterTracksMoved);
	connect(ui->splitter_genre, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitterGenreMoved);

	connect(ui->tv_albums, &ItemView::sigReloadClicked, this, &GUI_LocalLibrary::reloadLibraryRequested);
	connect(ui->tv_artists, &ItemView::sigReloadClicked, this, &GUI_LocalLibrary::reloadLibraryRequested);
	connect(ui->tv_tracks, &ItemView::sigReloadClicked, this, &GUI_LocalLibrary::reloadLibraryRequested);

	ui->extension_bar->init(m->library);
	ui->lv_genres->init(m->library);

	ListenSetting(Set::Lib_ShowAlbumCovers, GUI_LocalLibrary::switchAlbumView);
	ListenSetting(Set::Lib_ShowFilterExtBar, GUI_LocalLibrary::tracksLoaded);

	m->library->load();	
}

GUI_LocalLibrary::~GUI_LocalLibrary()
{
	delete ui; ui = nullptr;
}

void GUI_LocalLibrary::languageChanged()
{
	ui->retranslateUi(this);
	ui->gb_genres->setTitle(Lang::get(Lang::Genres));
	ui->btn_scanForFiles->setText(Lang::get(Lang::ScanForFiles));
	ui->btn_importDirectories->setText(Lang::get(Lang::ImportDir));

	GUI_AbstractLibrary::languageChanged();
}

void GUI_LocalLibrary::skinChanged()
{
	GUI_AbstractLibrary::skinChanged();

	checkViewState();
}

void GUI_LocalLibrary::checkViewState()
{
	if(this->isVisible())
	{
		checkReloadStatus();

		if(!m->library->isReloading())
		{
			checkFileExtensionBar();
		}
	}
}

void GUI_LocalLibrary::checkReloadStatus()
{
	bool isReloading = m->library->isReloading();
	bool isLibraryEmpty = m->library->isEmpty();

	ReloadWidgetIndex index = (isLibraryEmpty == false) ? ReloadWidgetIndex::StandardView : ReloadWidgetIndex::ReloadView;
	bool inLibraryState = (index == ReloadWidgetIndex::StandardView);

	ui->sw_reload->setCurrentIndex( int(index) );

	ui->pb_progress->setVisible(isReloading);
	ui->lab_progress->setVisible(isReloading);
	ui->widget_reload->setVisible(isReloading || isLibraryEmpty);

	ui->le_search->setVisible(inLibraryState);
	ui->btn_scanForFiles->setVisible(!inLibraryState);
	ui->btn_importDirectories->setVisible(!inLibraryState);

	m->library_menu->setLibraryEmpty(isLibraryEmpty);
}

void GUI_LocalLibrary::checkFileExtensionBar()
{
	ui->extension_bar->refresh();
	ui->extension_bar->setVisible
	(
		GetSetting(Set::Lib_ShowFilterExtBar) &&
		ui->extension_bar->hasExtensions()
	);
}


void GUI_LocalLibrary::tracksLoaded()
{
	checkViewState();

	ui->lab_libraryName->setText(m->library->name());
	ui->lab_path->setText(Util::createLink(m->library->path(), Style::isDark()));

	ui->btn_scanForFiles->setIcon(Gui::Icons::icon(Gui::Icons::Refresh));
	ui->btn_importDirectories->setIcon(Gui::Icons::icon(Gui::Icons::Folder));
}

void GUI_LocalLibrary::clearSelections()
{
	GUI_AbstractLibrary::clearSelections();

	if(ui->cover_view) {
		ui->cover_view->clearSelections();
	}

	ui->lv_genres->clearSelection();
}

void GUI_LocalLibrary::invalidGenreSelected()
{
	ui->le_search->setInvalidGenreMode(true);
	ui->le_search->setCurrentMode(Filter::Genre);
	ui->le_search->setText(GenreView::invalidGenreName());

	searchTriggered();

	ui->le_search->setInvalidGenreMode(false);
}

void GUI_LocalLibrary::genreSelectionChanged(const QStringList& genres)
{
	if(genres.isEmpty()) {
		return;
	}

	ui->le_search->setInvalidGenreMode(false);
	ui->le_search->setCurrentMode(Filter::Genre);
	ui->le_search->setText(genres.join(","));

	searchTriggered();
}


TrackDeletionMode GUI_LocalLibrary::showDeleteDialog(int track_count)
{
	GUI_DeleteDialog dialog(track_count, this);
	dialog.exec();

	return dialog.answer();
}


void GUI_LocalLibrary::progressChanged(const QString& type, int progress)
{
	checkViewState();

	QFontMetrics fm(this->font());

	ui->pb_progress->setMaximum((progress > 0) ? 100 : 0);
	ui->pb_progress->setValue(progress);
	ui->lab_progress->setText
	(
		fm.elidedText(type, Qt::ElideRight, ui->widget_reload->width() / 2)
	);
}

void GUI_LocalLibrary::reloadLibraryRequested()
{
	reloadLibraryRequestedWithQuality(ReloadQuality::Unknown);
}

void GUI_LocalLibrary::reloadLibraryDeepRequested()
{
	reloadLibraryRequestedWithQuality(ReloadQuality::Accurate);
}

void GUI_LocalLibrary::reloadLibraryRequestedWithQuality(ReloadQuality quality)
{
	if(quality == ReloadQuality::Unknown)
	{
		auto* dialog = new GUI_LibraryReloadDialog(m->library->name(), this);
		connect(dialog, &GUI_LibraryReloadDialog::sigAccepted, this, &GUI_LocalLibrary::reloadLibraryAccepted);

		dialog->setQuality(quality);
		dialog->show();
	}

	else
	{
		reloadLibrary(quality);
	}
}

void GUI_LocalLibrary::reloadLibraryAccepted(ReloadQuality quality)
{
	if(sender()) {
		sender()->deleteLater();
	}

	reloadLibrary(quality);
}


void GUI_LocalLibrary::reloadLibrary(ReloadQuality quality)
{
	m->library_menu->setLibraryBusy(true);
	m->library->reloadLibrary(false, quality);
}


void GUI_LocalLibrary::reloadFinished()
{
	m->library_menu->setLibraryBusy(false);

	checkViewState();
}

void GUI_LocalLibrary::showInfoBox()
{
	GUI_LibraryInfoBox(m->library->id(), this).exec();
}

void GUI_LocalLibrary::importDirsRequested()
{
	DirChooserDialog dialog(this);

	QStringList dirs;
	if(dialog.exec() == QFileDialog::Accepted){
		dirs = dialog.selectedFiles();
	}

	m->library->importFiles(dirs);
}

void GUI_LocalLibrary::importFilesRequested()
{
	QStringList files = QFileDialog::getOpenFileNames
	(
		this,
		Lang::get(Lang::ImportFiles),
		QDir::homePath(),
		Util::getFileFilter(Util::Extensions(Util::Extension::Soundfile), tr("Audio files"))
	);

	m->library->importFiles(files);
}


void GUI_LocalLibrary::nameChanged(LibraryId id)
{
	if(m->library->id() != id) {
		return;
	}

	Info info = Manager::instance()->libraryInfo(id);
	if(info.valid())
	{
		m->library_menu->refreshName(info.name());
		ui->lab_libraryName->setText(info.name());
	}
}


void GUI_LocalLibrary::pathChanged(LibraryId id)
{
	if(m->library->id() != id) {
		return;
	}

	Info info = Manager::instance()->libraryInfo(id);
	if(info.valid())
	{
		m->library_menu->refreshPath(info.path());

		if(this->isVisible())
		{
			reloadLibraryRequestedWithQuality(ReloadQuality::Accurate);
			ui->lab_path->setText(info.path());
		}
	}
}

void GUI_LocalLibrary::importDialogRequested(const QString& targetDirectory)
{
	if(!this->isVisible()){
		return;
	}

	auto* ui_importer = new GUI_ImportDialog(m->library, true, this);
	ui_importer->setTargetDirectory(targetDirectory);

	connect(ui_importer, &Gui::Dialog::sigClosed, ui_importer, &QObject::deleteLater);
	ui_importer->show();
}

void GUI_LocalLibrary::splitterArtistMoved(int pos, int idx)
{
	Q_UNUSED(pos) Q_UNUSED(idx)

	QByteArray arr = ui->splitter_artistAlbum->saveState();
	SetSetting(Set::Lib_SplitterStateArtist, arr);
}

void GUI_LocalLibrary::splitterTracksMoved(int pos, int idx)
{
	Q_UNUSED(pos) Q_UNUSED(idx)

	QByteArray arr = ui->splitter_tracks->saveState();
	SetSetting(Set::Lib_SplitterStateTrack, arr);
}

void GUI_LocalLibrary::splitterGenreMoved(int pos, int idx)
{
	Q_UNUSED(pos) Q_UNUSED(idx)

	QByteArray arr = ui->splitter_genre->saveState();
	SetSetting(Set::Lib_SplitterStateGenre, arr);
}


void GUI_LocalLibrary::switchAlbumView()
{
	bool show_covers = GetSetting(Set::Lib_ShowAlbumCovers);
	if(!show_covers)
	{
		ui->sw_albumCovers->setCurrentIndex(AlbumViewIndex::ArtistAlbumTableView);
	}

	else
	{
		if(!ui->cover_view->isInitialized())
		{
			ui->cover_view->init(m->library);
			connect(ui->cover_view, &GUI_CoverView::sigDeleteClicked, this, &GUI_LocalLibrary::itemDeleteClicked);
			connect(ui->cover_view, &GUI_CoverView::sigReloadClicked, this, &GUI_LocalLibrary::reloadLibraryRequested);
		}

		if(m->library->isLoaded() && (!m->library->selectedArtists().isEmpty()))
		{
			m->library->selectedArtistsChanged(IndexSet());
		}

		ui->sw_albumCovers->setCurrentIndex(AlbumViewIndex::AlbumCoverView);
	}
}

bool GUI_LocalLibrary::hasSelections() const
{
	return GUI_AbstractLibrary::hasSelections() ||
			(!ui->lv_genres->selectedItems().isEmpty()) ||
			(!ui->cover_view->selectedItems().isEmpty());
}

QList<Filter::Mode> GUI_LocalLibrary::searchOptions() const
{
	return { Filter::Fulltext, Filter::Filename, Filter::Genre };
}

void GUI_LocalLibrary::showEvent(QShowEvent* e)
{
	GUI_AbstractLibrary::showEvent(e);

	this->lvAlbum()->resizeRowsToContents();
	this->lvArtist()->resizeRowsToContents();
	this->lvTracks()->resizeRowsToContents();

	QMap<QSplitter*, QByteArray> splitters
	{
		{ui->splitter_artistAlbum, GetSetting(Set::Lib_SplitterStateArtist)},
		{ui->splitter_tracks, GetSetting(Set::Lib_SplitterStateTrack)},
		{ui->splitter_genre, GetSetting(Set::Lib_SplitterStateGenre)}
	};

	for(auto it=splitters.begin(); it != splitters.end(); it++)
	{
		if(!it.value().isEmpty()) {
			it.key()->restoreState(it.value());
		}
	}

	checkViewState();
}

// GUI_AbstractLibrary
TableView* GUI_LocalLibrary::lvArtist() const { return ui->tv_artists; }
TableView* GUI_LocalLibrary::lvAlbum() const { return ui->tv_albums; }
TableView* GUI_LocalLibrary::lvTracks() const { return ui->tv_tracks; }
SearchBar* GUI_LocalLibrary::leSearch() const { return ui->le_search; }

// LocalLibraryContainer
QMenu* GUI_LocalLibrary::menu() const {	return m->library_menu; }
QFrame* GUI_LocalLibrary::headerFrame() const { return ui->header_frame; }
