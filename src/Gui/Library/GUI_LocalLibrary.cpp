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
#include "GUI_ImportDialog.h"

#include "Gui/Library/Utils/GUI_DeleteDialog.h"
#include "Gui/Library/ui_GUI_LocalLibrary.h"

#include "Gui/Library/CoverView/GUI_CoverView.h"
#include "Gui/Library/Utils/DirChooserDialog.h"
#include "Gui/Library/Utils/GUI_ReloadLibraryDialog.h"
#include "Gui/Library/Utils/GUI_LibraryInfoBox.h"
#include "Gui/Library/Utils/LocalLibraryMenu.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Set.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QFileDialog>
#include <QStringList>
#include <QFileSystemWatcher>

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
	AlbumCoverView=1,
	DirectoryView=2
};

enum ReloadWidgetIndex
{
	TableView=0,
	ReloadView=1,
	NoDirView=2
};

using namespace Library;

struct GUI_LocalLibrary::Private
{
	LocalLibrary*			library = nullptr;
	LocalLibraryMenu*		libraryMenu = nullptr;

	Private(LibraryId id, GUI_LocalLibrary* parent)
	{
		library = Manager::instance()->libraryInstance(id);
		libraryMenu = new LocalLibraryMenu(library->name(), library->path(), parent);
	}
};


GUI_LocalLibrary::GUI_LocalLibrary(LibraryId id, QWidget* parent) :
	GUI_AbstractLibrary(Manager::instance()->libraryInstance(id), parent)
{
	m = Pimpl::make<Private>(id, this);

	setupParent(this, &ui);
	this->setFocusProxy(ui->leSearch);

	connect(m->library, &LocalLibrary::sigReloadingLibrary, this, &GUI_LocalLibrary::progressChanged);
	connect(m->library, &LocalLibrary::sigReloadingLibraryFinished, this, &GUI_LocalLibrary::reloadFinished);
	connect(m->library, &LocalLibrary::sigReloadingLibraryFinished, ui->lvGenres, &GenreView::reloadGenres);
	connect(m->library, &LocalLibrary::sigAllTracksLoaded, this, &GUI_LocalLibrary::tracksLoaded);
	connect(m->library, &LocalLibrary::sigImportDialogRequested, this, &GUI_LocalLibrary::importDialogRequested);

	auto* manager = Manager::instance();
	connect(manager, &Manager::sigPathChanged, this, &GUI_LocalLibrary::pathChanged);
	connect(manager, &Manager::sigRenamed, this, &GUI_LocalLibrary::nameChanged);

	connect(ui->tvAlbums, &AlbumView::sigDiscPressed, m->library, &LocalLibrary::changeCurrentDisc);
	connect(ui->lvGenres, &GenreView::sigSelectedChanged, this, &GUI_LocalLibrary::genreSelectionChanged);
	connect(ui->lvGenres, &GenreView::sigInvalidGenreSelected, this, &GUI_LocalLibrary::invalidGenreSelected);
	connect(ui->lvGenres, &GenreView::sigProgress, this, &GUI_LocalLibrary::progressChanged);

	connect(m->libraryMenu, &LocalLibraryMenu::sigPathChanged, m->library, &LocalLibrary::setLibraryPath);
	connect(m->libraryMenu, &LocalLibraryMenu::sigNameChanged, m->library, &LocalLibrary::setLibraryName);
	connect(m->libraryMenu, &LocalLibraryMenu::sigImportFile, this, &GUI_LocalLibrary::importFilesRequested);
	connect(m->libraryMenu, &LocalLibraryMenu::sigImportFolder, this, &GUI_LocalLibrary::importDirsRequested);
	connect(m->libraryMenu, &LocalLibraryMenu::sigInfo, this, &GUI_LocalLibrary::showInfoBox);
	connect(m->libraryMenu, &LocalLibraryMenu::sigReloadLibrary, this, &GUI_LocalLibrary::reloadLibraryRequested);

	connect(ui->btnScanForFiles, &QPushButton::clicked, this, &GUI_LocalLibrary::reloadLibraryDeepRequested);
	connect(ui->btnImportDirectories, &QPushButton::clicked, this, &GUI_LocalLibrary::importDirsRequested);

	connect(ui->splitterArtistAlbum, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitterArtistMoved);
	connect(ui->splitter_tracks, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitterTracksMoved);
	connect(ui->splitter_genre, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitterGenreMoved);

	connect(ui->tvAlbums, &ItemView::sigReloadClicked, this, &GUI_LocalLibrary::reloadLibraryRequested);
	connect(ui->tvArtists, &ItemView::sigReloadClicked, this, &GUI_LocalLibrary::reloadLibraryRequested);
	connect(ui->tvTracks, &ItemView::sigReloadClicked, this, &GUI_LocalLibrary::reloadLibraryRequested);

	ui->extensionBar->init(m->library);
	ui->lvGenres->init(m->library);
	ui->directoryView->setCurrentLibrary(m->library->id());
	ui->btnView->setOverrideText(true);

	ListenSetting(Set::Lib_ViewType, GUI_LocalLibrary::switchViewType);
	ListenSetting(Set::Lib_ShowFilterExtBar, GUI_LocalLibrary::tracksLoaded);

	auto* sch = ShortcutHandler::instance();
	Shortcut sc = sch->shortcut(ShortcutIdentifier::CoverView);
	sc.connect(this, [this]() {
		this->selectNextViewType();
	});

	const QStringList paths
	{
		m->library->path(),
		Util::File::getParentDirectory(m->library->path())
	};

	auto* action = new Gui::LibraryPreferenceAction(this);
	connect(ui->btnLibraryPreferences, &QPushButton::clicked, action, &QAction::trigger);

	auto* fileSystemWatcher = new QFileSystemWatcher(paths, this);
	connect(fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString& path){
		Q_UNUSED(path)
		this->checkMainSplitterStatus();
	});
}

GUI_LocalLibrary::~GUI_LocalLibrary()
{
	delete ui; ui = nullptr;
}

void GUI_LocalLibrary::checkViewState()
{
	if(this->isVisible())
	{
		checkMainSplitterStatus();

		if(!m->library->isReloading())
		{
			checkFileExtensionBar();
		}
	}
}

void GUI_LocalLibrary::checkMainSplitterStatus()
{
	bool pathExists = Util::File::exists(m->library->path());
	bool isLibraryEmpty = m->library->isEmpty();

	ReloadWidgetIndex index = ReloadWidgetIndex::TableView;
	if(!pathExists) {
		index = ReloadWidgetIndex::NoDirView;
	}

	else if(isLibraryEmpty) {
		index = ReloadWidgetIndex::ReloadView;
	}

	ui->swReload->setCurrentIndex( int(index) );

	bool inLibraryState = (index == ReloadWidgetIndex::TableView);

	ui->leSearch->setVisible(inLibraryState);
	ui->btnScanForFiles->setVisible(!inLibraryState);
	ui->btnImportDirectories->setVisible(!inLibraryState);

	if(index == ReloadWidgetIndex::NoDirView)
	{
		ui->labDir->setText(m->library->path());
	}

	else
	{
		bool isReloading = m->library->isReloading();

		ui->pbProgress->setVisible(isReloading);
		ui->labProgress->setVisible(isReloading);
		ui->widgetReload->setVisible(isReloading || isLibraryEmpty);
		m->libraryMenu->setLibraryEmpty(isLibraryEmpty);
	}
}

void GUI_LocalLibrary::checkFileExtensionBar()
{
	ui->extensionBar->refresh();
	ui->extensionBar->setVisible
	(
		GetSetting(Set::Lib_ShowFilterExtBar) &&
		ui->extensionBar->hasExtensions()
	);
}

void GUI_LocalLibrary::tracksLoaded()
{
	checkViewState();

	ui->labLibraryName->setText(m->library->name());
	ui->labPath->setText(Util::createLink(m->library->path(), Style::isDark()));

	ui->btnScanForFiles->setIcon(Gui::Icons::icon(Gui::Icons::Refresh));
	ui->btnImportDirectories->setIcon(Gui::Icons::icon(Gui::Icons::Folder));
}

void GUI_LocalLibrary::clearSelections()
{
	GUI_AbstractLibrary::clearSelections();

	if(ui->coverView) {
		ui->coverView->clearSelections();
	}

	ui->lvGenres->clearSelection();
}

void GUI_LocalLibrary::invalidGenreSelected()
{
	ui->leSearch->setInvalidGenreMode(true);
	ui->leSearch->setCurrentMode(Filter::Genre);
	ui->leSearch->setText(GenreView::invalidGenreName());

	searchTriggered();

	ui->leSearch->setInvalidGenreMode(false);
}

void GUI_LocalLibrary::genreSelectionChanged(const QStringList& genres)
{
	if(genres.isEmpty()) {
		return;
	}

	ui->leSearch->setInvalidGenreMode(false);
	ui->leSearch->setCurrentMode(Filter::Genre);
	ui->leSearch->setText(genres.join(","));

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

	ui->pbProgress->setMaximum((progress > 0) ? 100 : 0);
	ui->pbProgress->setValue(progress);
	ui->labProgress->setText
	(
		fm.elidedText(type, Qt::ElideRight, ui->widgetReload->width() / 2)
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
	m->libraryMenu->setLibraryBusy(true);
	m->library->reloadLibrary(false, quality);
}


void GUI_LocalLibrary::reloadFinished()
{
	m->libraryMenu->setLibraryBusy(false);

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
		nullptr,
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
		m->libraryMenu->refreshName(info.name());
		ui->labLibraryName->setText(info.name());
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
		m->libraryMenu->refreshPath(info.path());

		if(this->isVisible())
		{
			reloadLibraryRequestedWithQuality(ReloadQuality::Accurate);
			ui->labPath->setText(info.path());
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

	QByteArray arr = ui->splitterArtistAlbum->saveState();
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

void GUI_LocalLibrary::switchViewType()
{
	Library::ViewType viewType = GetSetting(Set::Lib_ViewType);
	switch(viewType)
	{
		case Library::ViewType::CoverView:
			if(!ui->coverView->isInitialized())
			{
				ui->coverView->init(m->library);
				connect(ui->coverView, &GUI_CoverView::sigDeleteClicked, this, &GUI_LocalLibrary::itemDeleteClicked);
				connect(ui->coverView, &GUI_CoverView::sigReloadClicked, this, &GUI_LocalLibrary::reloadLibraryRequested);
			}

			if(m->library->isLoaded() && (!m->library->selectedArtists().isEmpty()))
			{
				m->library->selectedArtistsChanged(IndexSet());
			}

			ui->swViewType->setCurrentIndex(AlbumViewIndex::AlbumCoverView);
			break;

		case Library::ViewType::FileView:
			ui->swViewType->setCurrentIndex(AlbumViewIndex::DirectoryView);
			break;

		case Library::ViewType::Standard:
		default:
			//ui->swViewType->setFocusProxy(ui->tvArtists);
			ui->swViewType->setCurrentIndex(AlbumViewIndex::ArtistAlbumTableView);
			break;
	}

	ui->swViewType->setFocus();
}

void GUI_LocalLibrary::selectNextViewType()
{
	int vt = int(GetSetting(Set::Lib_ViewType));
	vt = (vt + 1) % 3;
	SetSetting(Set::Lib_ViewType, ViewType(vt));
}

bool GUI_LocalLibrary::hasSelections() const
{
	return GUI_AbstractLibrary::hasSelections() ||
			(!ui->lvGenres->selectedItems().isEmpty()) ||
			(!ui->coverView->selectedItems().isEmpty());
}

QList<Filter::Mode> GUI_LocalLibrary::searchOptions() const
{
	return { Filter::Fulltext, Filter::Filename, Filter::Genre };
}

void GUI_LocalLibrary::queryLibrary()
{
	GUI_AbstractLibrary::queryLibrary();
	ui->directoryView->setFilterTerm(m->library->filter().filtertext(false).join(""));
}

#include <QTimer>
void GUI_LocalLibrary::showEvent(QShowEvent* e)
{
	GUI_AbstractLibrary::showEvent(e);

	const QMap<QSplitter*, QByteArray> splitters
	{
		{ui->splitterArtistAlbum, GetSetting(Set::Lib_SplitterStateArtist)},
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

	QTimer::singleShot(1000, this, [this](){
		m->library->load();
	});
}

void GUI_LocalLibrary::languageChanged()
{
	ui->retranslateUi(this);

	ui->btnLibraryPreferences->setText(Lang::get(Lang::Preferences));
	ui->gbGenres->setTitle(Lang::get(Lang::Genres));
	ui->btnScanForFiles->setText(Lang::get(Lang::ScanForFiles));
	ui->btnImportDirectories->setText(Lang::get(Lang::ImportDir));

	GUI_AbstractLibrary::languageChanged();
}

void GUI_LocalLibrary::skinChanged()
{
	GUI_AbstractLibrary::skinChanged();

	checkViewState();
}

// GUI_AbstractLibrary
Library::TableView* GUI_LocalLibrary::lvArtist() const { return ui->tvArtists; }
Library::TableView* GUI_LocalLibrary::lvAlbum() const { return ui->tvAlbums; }
Library::TableView* GUI_LocalLibrary::lvTracks() const { return ui->tvTracks; }
Library::SearchBar* GUI_LocalLibrary::leSearch() const { return ui->leSearch; }

// LocalLibraryContainer
QMenu* GUI_LocalLibrary::menu() const {	return m->libraryMenu; }
QFrame* GUI_LocalLibrary::headerFrame() const { return ui->header_frame; }
