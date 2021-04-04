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

#include <QDir>
#include <QFileDialog>
#include <QStringList>
#include <QFileSystemWatcher>

namespace
{
	// ui->swViewType
	enum AlbumViewIndex
	{
		ArtistAlbumTableView = 0,
		AlbumCoverView = 1,
		DirectoryView = 2
	};

	// ui->swReload
	enum ReloadWidgetIndex
	{
		TableView = 0,
		ReloadView = 1,
		NoDirView = 2
	};

	ReloadWidgetIndex getReloadWidgetIndex(const Library::Info& libraryInfo, bool isLibraryEmpty)
	{
		const auto pathExists = Util::File::exists(libraryInfo.path());
		if(!pathExists)
		{
			return ReloadWidgetIndex::NoDirView;
		}

		else if(isLibraryEmpty)
		{
			return ReloadWidgetIndex::ReloadView;
		}

		return ReloadWidgetIndex::TableView;
	}
}

using namespace Library;

struct GUI_LocalLibrary::Private
{
	LocalLibrary* library;
	LocalLibraryMenu* libraryMenu;

	Private(LocalLibrary* localLibrary, GUI_LocalLibrary* parent) :
		library {localLibrary},
		libraryMenu {new LocalLibraryMenu(library->info().name(), library->info().path(), parent)} {}
};

GUI_LocalLibrary::GUI_LocalLibrary(LibraryId id, Library::Manager* libraryManager, QWidget* parent) :
	GUI_AbstractLibrary(libraryManager->libraryInstance(id), parent)
{
	m = Pimpl::make<Private>(libraryManager->libraryInstance(id), this);

	setupParent(this, &ui);
	this->setFocusProxy(ui->leSearch);

	ui->directoryView->init(libraryManager, id);

	connect(m->library, &LocalLibrary::sigReloadingLibrary, this, &GUI_LocalLibrary::progressChanged);
	connect(m->library, &LocalLibrary::sigReloadingLibraryFinished, this, &GUI_LocalLibrary::reloadFinished);
	connect(m->library, &LocalLibrary::sigReloadingLibraryFinished, ui->lvGenres, &GenreView::reloadGenres);
	connect(m->library, &LocalLibrary::sigAllTracksLoaded, this, &GUI_LocalLibrary::tracksLoaded);
	connect(m->library, &LocalLibrary::sigImportDialogRequested, this, &GUI_LocalLibrary::importDialogRequested);
	connect(m->library, &LocalLibrary::sigRenamed, this, &GUI_LocalLibrary::nameChanged);
	connect(m->library, &LocalLibrary::sigPathChanged, this, &GUI_LocalLibrary::pathChanged);

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
	connect(ui->splitterTracks, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitterTracksMoved);
	connect(ui->splitterGenre, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitterGenreMoved);

	connect(ui->tvAlbums, &ItemView::sigReloadClicked, this, &GUI_LocalLibrary::reloadLibraryRequested);
	connect(ui->tvArtists, &ItemView::sigReloadClicked, this, &GUI_LocalLibrary::reloadLibraryRequested);
	connect(ui->tvTracks, &ItemView::sigReloadClicked, this, &GUI_LocalLibrary::reloadLibraryRequested);

	ui->extensionBar->init(m->library);
	ui->lvGenres->init(m->library);
	ui->btnView->setOverrideText(true);

	ListenSetting(Set::Lib_ViewType, GUI_LocalLibrary::switchViewType);
	ListenSetting(Set::Lib_ShowFilterExtBar, GUI_LocalLibrary::tracksLoaded);

	auto* shortcutHandler = ShortcutHandler::instance();
	auto shortcut = shortcutHandler->shortcut(ShortcutIdentifier::CoverView);
	shortcut.connect(this, [this]() {
		this->selectNextViewType();
	});

	const auto libraryPath = m->library->info().path();
	const auto paths = {
		libraryPath,
		Util::File::getParentDirectory(libraryPath)
	};

	auto* action = new Gui::LibraryPreferenceAction(this);
	connect(ui->btnLibraryPreferences, &QPushButton::clicked, action, &QAction::trigger);

	auto* fileSystemWatcher = new QFileSystemWatcher(paths, this);
	connect(fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, [this](const auto& /*path*/) {
		this->checkMainSplitterStatus();
	});
}

GUI_LocalLibrary::~GUI_LocalLibrary()
{
	delete ui;
	ui = nullptr;
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
	const auto isLibraryEmpty = m->library->isEmpty();
	const auto libraryInfo = m->library->info();

	const auto index = getReloadWidgetIndex(libraryInfo, isLibraryEmpty);
	ui->swReload->setCurrentIndex(static_cast<int>(index));

	const auto inLibraryState = (index == ReloadWidgetIndex::TableView);
	ui->leSearch->setVisible(inLibraryState);
	ui->btnScanForFiles->setVisible(!inLibraryState);
	ui->btnImportDirectories->setVisible(!inLibraryState);

	if(index == ReloadWidgetIndex::NoDirView)
	{
		ui->labDir->setText(libraryInfo.path());
	}

	else
	{
		const auto isReloading = m->library->isReloading();

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

	const auto info = m->library->info();

	ui->labLibraryName->setText(info.name());
	ui->labPath->setText(Util::createLink(info.path(), Style::isDark()));

	ui->btnScanForFiles->setIcon(Gui::Icons::icon(Gui::Icons::Refresh));
	ui->btnImportDirectories->setIcon(Gui::Icons::icon(Gui::Icons::Folder));
}

void GUI_LocalLibrary::clearSelections()
{
	GUI_AbstractLibrary::clearSelections();

	if(ui->coverView)
	{
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
	if(!genres.isEmpty())
	{
		ui->leSearch->setInvalidGenreMode(false);
		ui->leSearch->setCurrentMode(Filter::Genre);
		ui->leSearch->setText(genres.join(","));

		searchTriggered();
	}
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

	ui->pbProgress->setMaximum((progress > 0) ? 100 : 0);
	ui->pbProgress->setValue(progress);
	ui->labProgress->setText
		(
			this->fontMetrics().elidedText(type, Qt::ElideRight, ui->widgetReload->width() / 2)
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
		auto* dialog = new GUI_LibraryReloadDialog(m->library->info().name(), this);
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
	if(sender())
	{
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
	const auto info = m->library->info();
	GUI_LibraryInfoBox infoBox(info, this);
	infoBox.exec();
}

void GUI_LocalLibrary::importDirsRequested()
{
	DirChooserDialog dialog(this);

	if(dialog.exec() == QFileDialog::Accepted)
	{
		const auto dirs = dialog.selectedFiles();
		m->library->importFiles(dirs);
	}
}

void GUI_LocalLibrary::importFilesRequested()
{
	const auto files = QFileDialog::getOpenFileNames
		(
			nullptr,
			Lang::get(Lang::ImportFiles),
			QDir::homePath(),
			Util::getFileFilter(Util::Extensions(Util::Extension::Soundfile), tr("Audio files"))
		);

	m->library->importFiles(files);
}

void GUI_LocalLibrary::nameChanged(const QString& newName)
{
	m->libraryMenu->refreshName(newName);
	ui->labLibraryName->setText(newName);
}

void GUI_LocalLibrary::pathChanged(const QString& newPath)
{
	m->libraryMenu->refreshPath(newPath);

	if(this->isVisible())
	{
		reloadLibraryRequestedWithQuality(ReloadQuality::Accurate);
		ui->labPath->setText(newPath);
	}
}

void GUI_LocalLibrary::importDialogRequested(const QString& targetDirectory)
{
	if(this->isVisible())
	{
		auto* uiImporter = new GUI_ImportDialog(m->library, true, this);
		uiImporter->setTargetDirectory(targetDirectory);

		connect(uiImporter, &Gui::Dialog::sigClosed, uiImporter, &QObject::deleteLater);
		uiImporter->show();
	}
}

void GUI_LocalLibrary::splitterArtistMoved([[maybe_unused]] int pos, [[maybe_unused]] int idx)
{
	const auto data = ui->splitterArtistAlbum->saveState();
	SetSetting(Set::Lib_SplitterStateArtist, data);
}

void GUI_LocalLibrary::splitterTracksMoved([[maybe_unused]] int pos, [[maybe_unused]] int idx)
{
	const auto data = ui->splitterTracks->saveState();
	SetSetting(Set::Lib_SplitterStateTrack, data);
}

void GUI_LocalLibrary::splitterGenreMoved([[maybe_unused]] int pos, [[maybe_unused]] int idx)
{
	const auto data = ui->splitterGenre->saveState();
	SetSetting(Set::Lib_SplitterStateGenre, data);
}

void GUI_LocalLibrary::switchViewType()
{
	const auto viewType = GetSetting(Set::Lib_ViewType);
	switch(viewType)
	{
		case Library::ViewType::CoverView:
			if(!ui->coverView->isInitialized())
			{
				ui->coverView->init(m->library);
				connect(ui->coverView, &GUI_CoverView::sigDeleteClicked, this, &GUI_LocalLibrary::itemDeleteClicked);
				connect(ui->coverView,
				        &GUI_CoverView::sigReloadClicked,
				        this,
				        &GUI_LocalLibrary::reloadLibraryRequested);
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
			ui->swViewType->setCurrentIndex(AlbumViewIndex::ArtistAlbumTableView);
			break;
	}

	ui->swViewType->setFocus();
}

void GUI_LocalLibrary::selectNextViewType()
{
	auto viewType = static_cast<int>(GetSetting(Set::Lib_ViewType));
	viewType = (viewType + 1) % 3;
	SetSetting(Set::Lib_ViewType, ViewType(viewType));
}

bool GUI_LocalLibrary::hasSelections() const
{
	return GUI_AbstractLibrary::hasSelections() ||
	       (!ui->lvGenres->selectedItems().isEmpty()) ||
	       (!ui->coverView->selectedItems().isEmpty());
}

QList<Filter::Mode> GUI_LocalLibrary::searchOptions() const
{
	return {Filter::Fulltext, Filter::Filename, Filter::Genre};
}

void GUI_LocalLibrary::queryLibrary()
{
	GUI_AbstractLibrary::queryLibrary();
	ui->directoryView->setFilterTerm(m->library->filter().filtertext(false).join(""));
}

void GUI_LocalLibrary::showEvent(QShowEvent* e)
{
	GUI_AbstractLibrary::showEvent(e);

	const QMap<QSplitter*, QByteArray> splitters
		{
			{ui->splitterArtistAlbum, GetSetting(Set::Lib_SplitterStateArtist)},
			{ui->splitterTracks,      GetSetting(Set::Lib_SplitterStateTrack)},
			{ui->splitterGenre,       GetSetting(Set::Lib_SplitterStateGenre)}
		};

	for(auto it = splitters.begin(); it != splitters.end(); it++)
	{
		if(!it.value().isEmpty())
		{
			it.key()->restoreState(it.value());
		}
	}

	checkViewState();
	m->library->load();
}

void GUI_LocalLibrary::languageChanged()
{
	ui->retranslateUi(this);

	ui->btnLibraryPreferences->setText(Lang::get(Lang::Preferences));
	ui->labGenres->setText(Lang::get(Lang::Genres));
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

QList<QAbstractItemView*> GUI_LocalLibrary::allViews() const
{
	return {ui->lvGenres, ui->tvAlbums, ui->tvArtists, ui->tvTracks};
}

Library::SearchBar* GUI_LocalLibrary::leSearch() const { return ui->leSearch; }

// LocalLibraryContainer
QMenu* GUI_LocalLibrary::menu() const { return m->libraryMenu; }

QFrame* GUI_LocalLibrary::headerFrame() const { return ui->headerFrame; }
