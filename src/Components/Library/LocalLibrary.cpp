/* LocalLibrary.cpp */

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

#include "LocalLibrary.h"
#include "Importer/LibraryImporter.h"
#include "Threads/ReloadThread.h"

#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Database/Connector.h"
#include "Database/Library.h"
#include "Database/LibraryDatabase.h"

#include "Interfaces/LibraryPlaylistInteractor.h"

#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"

#include <QTime>

namespace
{
	template<typename T>
	QHash<Id, int> createIdRowMap(const T& items)
	{
		auto idRowMap = QHash<Id, int> {};
		for(auto it = items.begin(); it != items.end(); it++)
		{
			idRowMap[it->id()] = std::distance(items.begin(), it);
		}

		return idRowMap;
	}
}

struct LocalLibrary::Private
{
	LibraryId libraryId;
	DB::LibraryDatabase* libraryDatabase;

	Library::Manager* libraryManager;
	Library::ReloadThread* reloadThread = nullptr;
	Library::Importer* libraryImporter = nullptr;

	Private(Library::Manager* libraryManager, LibraryId libraryId) :
		libraryId(libraryId),
		libraryDatabase(DB::Connector::instance()->libraryDatabase(libraryId, 0)),
		libraryManager(libraryManager) {}
};

LocalLibrary::LocalLibrary(Library::Manager* libraryManager, LibraryId libraryId,
                           LibraryPlaylistInteractor* playlistInteractor, QObject* parent) :
	AbstractLibrary(playlistInteractor, parent),
	m {Pimpl::make<Private>(libraryManager, libraryId)}
{
	applyDatabaseFixes();

	connect(libraryManager, &Library::Manager::sigRenamed, this, [&](const auto id) {
		if(id == m->libraryId)
		{
			emit sigRenamed(info().name());
		}
	});

	connect(libraryManager, &Library::Manager::sigPathChanged, this, [&](const auto id) {
		if(id == m->libraryId)
		{
			emit sigPathChanged(info().path());
		}
	});

	ListenSettingNoCall(Set::Lib_SearchMode, LocalLibrary::searchModeChanged);
	ListenSettingNoCall(Set::Lib_ShowAlbumArtists, LocalLibrary::showAlbumArtistsChanged);
}

LocalLibrary::~LocalLibrary() = default;

void LocalLibrary::initLibraryImpl()
{
	auto* mdcn = Tagging::ChangeNotifier::instance();
	connect(mdcn, &Tagging::ChangeNotifier::sigMetadataChanged,
	        this, &LocalLibrary::metadataChanged);

	connect(mdcn, &Tagging::ChangeNotifier::sigMetadataDeleted,
	        this, &LocalLibrary::metadataChanged);

	// rating
	connect(mdcn, &Tagging::ChangeNotifier::sigAlbumsChanged,
	        this, &LocalLibrary::albumsChanged);
}

void LocalLibrary::metadataChanged()
{
	auto* mdcn = dynamic_cast<Tagging::ChangeNotifier*>(sender());

	const auto& changedTracks = mdcn->changedMetadata();
	const auto idRowMap = createIdRowMap(tracks());

	auto needsRefresh = false;
	for(const auto& [oldTrack, newTrack]: changedTracks)
	{
		needsRefresh = needsRefresh ||
			(oldTrack.albumArtistId() != newTrack.albumArtistId()) ||
			(oldTrack.albumId() != newTrack.albumId()) ||
			(oldTrack.artistId() != newTrack.artistId()) ||
			(oldTrack.album() != newTrack.album()) ||
			(oldTrack.albumArtist() != newTrack.albumArtist()) ||
			(oldTrack.artist() != newTrack.artist());

		if(idRowMap.contains(oldTrack.id()))
		{
			const auto row = idRowMap[oldTrack.id()];
			replaceTrack(row, newTrack);
		}
	}

	if(needsRefresh)
	{
		refreshCurrentView();
	}
}

void LocalLibrary::albumsChanged()
{
	auto* mdcn = dynamic_cast<Tagging::ChangeNotifier*>(sender());

	const auto& changedAlbums = mdcn->changedAlbums();
	const auto idRowMap = createIdRowMap(albums());

	for(const auto& [oldAlbum, newAlbum]: changedAlbums)
	{
		if(idRowMap.contains(oldAlbum.id()))
		{
			const auto row = idRowMap[oldAlbum.id()];
			replaceAlbum(row, newAlbum);
		}
	}
}

void LocalLibrary::applyDatabaseFixes() {}

void LocalLibrary::reloadLibrary(bool clearFirst, Library::ReloadQuality quality)
{
	if(isReloading())
	{
		return;
	}

	if(!m->reloadThread)
	{
		initReloadThread();
	}

	if(clearFirst)
	{
		deleteAllTracks();
	}

	const auto info = this->info();
	m->reloadThread->setLibrary(info.id(), info.path());
	m->reloadThread->setQuality(quality);
	m->reloadThread->start();
}

void LocalLibrary::reloadThreadFinished()
{
	refetch();

	emit sigReloadingLibrary(QString(), -1);
	emit sigReloadingLibraryFinished();
}

void LocalLibrary::searchModeChanged()
{
	spLog(Log::Debug, this) << "Updating cissearch... " << GetSetting(Set::Lib_SearchMode);

	m->libraryDatabase->updateSearchMode();

	spLog(Log::Debug, this) << "Updating cissearch finished" << GetSetting(Set::Lib_SearchMode);
}

void LocalLibrary::showAlbumArtistsChanged()
{
	const auto showAlbumArtists = GetSetting(Set::Lib_ShowAlbumArtists);

	const auto libraryDatabases = DB::Connector::instance()->libraryDatabases();
	for(auto* libraryDatabase: libraryDatabases)
	{
		if(libraryDatabase->databaseId() == 0)
		{
			const auto field = (showAlbumArtists)
			                   ? DB::LibraryDatabase::ArtistIDField::AlbumArtistID
			                   : DB::LibraryDatabase::ArtistIDField::ArtistID;

			libraryDatabase->changeArtistIdField(field);
		}
	}

	refreshCurrentView();
}

void LocalLibrary::importStatusChanged(Library::Importer::ImportStatus status)
{
	if(status == Library::Importer::ImportStatus::Imported)
	{
		refreshCurrentView();
	}
}

void LocalLibrary::reloadThreadNewBlock()
{
	m->reloadThread->pause();
	refreshCurrentView();
	m->reloadThread->goon();
}

void LocalLibrary::getAllArtists(ArtistList& artists) const
{
	m->libraryDatabase->getAllArtists(artists, false);
}

void LocalLibrary::getAllArtistsBySearchstring(Library::Filter filter, ArtistList& artists) const
{
	m->libraryDatabase->getAllArtistsBySearchString(filter, artists);
}

void LocalLibrary::getAllAlbums(AlbumList& albums) const
{
	m->libraryDatabase->getAllAlbums(albums, false);
}

void LocalLibrary::getAllAlbumsByArtist(IdList artistIds, AlbumList& albums, Library::Filter filter) const
{
	m->libraryDatabase->getAllAlbumsByArtist(artistIds, albums, filter);
}

void LocalLibrary::getAllAlbumsBySearchstring(Library::Filter filter, AlbumList& albums) const
{
	m->libraryDatabase->getAllAlbumsBySearchString(filter, albums);
}

int LocalLibrary::getTrackCount() const
{
	return m->libraryDatabase->getNumTracks();
}

void LocalLibrary::getAllTracks(MetaDataList& tracks) const
{
	m->libraryDatabase->getAllTracks(tracks);
}

void LocalLibrary::getAllTracks(const QStringList& paths, MetaDataList& tracks) const
{
	m->libraryDatabase->getMultipleTracksByPath(paths, tracks);
}

void LocalLibrary::getAllTracksByArtist(IdList artistIds, MetaDataList& tracks, Library::Filter filter) const
{
	m->libraryDatabase->getAllTracksByArtist(artistIds, tracks, filter);
}

void LocalLibrary::getAllTracksByAlbum(IdList albumIds, MetaDataList& tracks, Library::Filter filter) const
{
	m->libraryDatabase->getAllTracksByAlbum(albumIds, tracks, filter, -1);
}

void LocalLibrary::getAllTracksBySearchstring(Library::Filter filter, MetaDataList& tracks) const
{
	m->libraryDatabase->getAllTracksBySearchString(filter, tracks);
}

void LocalLibrary::getAllTracksByPath(const QStringList& paths, MetaDataList& tracks) const
{
	m->libraryDatabase->getAllTracksByPaths(paths, tracks);
}

void LocalLibrary::getTrackById(TrackID trackId, MetaData& track) const
{
	const auto tmpTrack = m->libraryDatabase->getTrackById(trackId);
	track = (tmpTrack.libraryId() == m->libraryId)
	        ? tmpTrack
	        : MetaData();
}

void LocalLibrary::getAlbumById(AlbumId albumId, Album& album) const
{
	m->libraryDatabase->getAlbumByID(albumId, album);
}

void LocalLibrary::getArtistById(ArtistId artistId, Artist& artist) const
{
	m->libraryDatabase->getArtistByID(artistId, artist);
}

void LocalLibrary::initReloadThread()
{
	if(!m->reloadThread)
	{
		m->reloadThread = new Library::ReloadThread(this);

		connect(m->reloadThread, &Library::ReloadThread::sigReloadingLibrary,
		        this, &LocalLibrary::sigReloadingLibrary);

		connect(m->reloadThread, &Library::ReloadThread::sigNewBlockSaved,
		        this, &LocalLibrary::reloadThreadNewBlock);

		connect(m->reloadThread, &Library::ReloadThread::finished,
		        this, &LocalLibrary::reloadThreadFinished);
	}
}

void LocalLibrary::deleteTracks(const MetaDataList& tracks, Library::TrackDeletionMode mode)
{
	m->libraryDatabase->deleteTracks(tracks.trackIds());
	AbstractLibrary::deleteTracks(tracks, mode);
}

void LocalLibrary::refreshArtists() {}

void LocalLibrary::refreshAlbums() {}

void LocalLibrary::refreshTracks() {}

void LocalLibrary::importFiles(const QStringList& files)
{
	importFilesTo(files, QString());
}

void LocalLibrary::importFilesTo(const QStringList& files, const QString& targetDirectory)
{
	if(!files.isEmpty())
	{
		if(!m->libraryImporter)
		{
			m->libraryImporter = new Library::Importer(this);
			connect(m->libraryImporter, &Library::Importer::sigStatusChanged, this, &LocalLibrary::importStatusChanged);
		}

		m->libraryImporter->importFiles(files, targetDirectory);

		emit sigImportDialogRequested(targetDirectory);
	}
}

bool LocalLibrary::setLibraryPath(const QString& library_path)
{
	return m->libraryManager->changeLibraryPath(m->libraryId, library_path);
}

bool LocalLibrary::setLibraryName(const QString& library_name)
{
	return m->libraryManager->renameLibrary(m->libraryId, library_name);
}

Library::Info LocalLibrary::info() const
{
	return m->libraryManager->libraryInfo(m->libraryId);
}

Library::Importer* LocalLibrary::importer()
{
	if(!m->libraryImporter)
	{
		m->libraryImporter = new Library::Importer(this);
		connect(m->libraryImporter, &Library::Importer::sigStatusChanged, this, &LocalLibrary::importStatusChanged);
	}

	return m->libraryImporter;
}

bool LocalLibrary::isReloading() const
{
	return (m->reloadThread != nullptr && m->reloadThread->isRunning());
}
