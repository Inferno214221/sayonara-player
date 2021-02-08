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

#include "Database/Connector.h"
#include "Database/Library.h"
#include "Database/LibraryDatabase.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"
#include "Utils/globals.h"
#include "Utils/Set.h"

#include <utility>
#include <limits>
#include <QTime>


struct LocalLibrary::Private
{
	Library::ReloadThread*	reloadThread=nullptr;
	Library::Importer*		libraryImporter=nullptr;

	LibraryId				libraryId;

	Private(LibraryId libraryId) :
		libraryId(libraryId)
	{}
};

LocalLibrary::LocalLibrary(LibraryId libraryId, Playlist::Handler* playlistHandler,  QObject* parent) :
	AbstractLibrary(playlistHandler, parent)
{
	m = Pimpl::make<Private>(libraryId);

	applyDatabaseFixes();

	connect(playlistHandler, &Playlist::Handler::sigTrackDeletionRequested,
			this, &LocalLibrary::deleteTracks);

	connect(playlistHandler, &Playlist::Handler::sigFindTrackRequested,
			this, &LocalLibrary::findTrack);

	auto* manager = Library::Manager::instance();
	connect(manager, &Library::Manager::sigRenamed, this, &LocalLibrary::renamed);

	ListenSettingNoCall(Set::Lib_SearchMode, LocalLibrary::searchModeChanged);
	ListenSettingNoCall(Set::Lib_ShowAlbumArtists, LocalLibrary::showAlbumArtistsChanged);
}

LocalLibrary::~LocalLibrary() = default;

void LocalLibrary::applyDatabaseFixes() {}

void LocalLibrary::reloadLibrary(bool clearFirst, Library::ReloadQuality quality)
{
	if(isReloading()){
		return;
	}

	if(!m->reloadThread){
		initReloadThread();
	}

	if(clearFirst) {
		deleteAllTracks();
	}

	m->reloadThread->setQuality(quality);
	m->reloadThread->setLibrary(id(), path());
	m->reloadThread->start();
}


void LocalLibrary::reloadThreadFinished()
{
	load();

	emit sigReloadingLibrary("", -1);
	emit sigReloadingLibraryFinished();
}

void LocalLibrary::searchModeChanged()
{
	spLog(Log::Debug, this) << "Updating cissearch... " << GetSetting(Set::Lib_SearchMode);

	auto* libDb = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	libDb->updateSearchMode();

	spLog(Log::Debug, this) << "Updating cissearch finished" << GetSetting(Set::Lib_SearchMode);
}

void LocalLibrary::showAlbumArtistsChanged()
{
	bool showAlbumArtists = GetSetting(Set::Lib_ShowAlbumArtists);

	DB::LibraryDatabases dbs = DB::Connector::instance()->libraryDatabases();
	for(DB::LibraryDatabase* libDb : dbs)
	{
		if(libDb->databaseId() == 0)
		{
			if(showAlbumArtists)
			{
				libDb->changeArtistIdField(DB::LibraryDatabase::ArtistIDField::AlbumArtistID);
			}

			else
			{
				libDb->changeArtistIdField(DB::LibraryDatabase::ArtistIDField::ArtistID);
			}
		}
	}

	refreshCurrentView();
}

void LocalLibrary::renamed(LibraryId id)
{
	if(id == this->id())
	{
		emit sigRenamed( this->name() );
	}
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
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getAllArtists(artists, false);
}

void LocalLibrary::getAllArtistsBySearchstring(Library::Filter filter, ArtistList& artists) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getAllArtistsBySearchString(filter, artists);
}

void LocalLibrary::getAllAlbums(AlbumList& albums) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getAllAlbums(albums, false);
}

void LocalLibrary::getAllAlbumsByArtist(IdList artistIds, AlbumList& albums, Library::Filter filter) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getAllAlbumsByArtist(artistIds, albums, filter);
}

void LocalLibrary::getAllAlbumsBySearchstring(Library::Filter filter, AlbumList& albums) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getAllAlbumsBySearchString(filter, albums);
}

int LocalLibrary::getTrackCount() const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	return lib_db->getNumTracks();
}

void LocalLibrary::getAllTracks(MetaDataList& v_md) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getAllTracks(v_md);
}

void LocalLibrary::getAllTracks(const QStringList& paths, MetaDataList& v_md) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getMultipleTracksByPath(paths, v_md);
}

void LocalLibrary::getAllTracksByArtist(IdList artistIds, MetaDataList& v_md, Library::Filter filter) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getAllTracksByArtist(artistIds, v_md, filter);
}

void LocalLibrary::getAllTracksByAlbum(IdList albumIds, MetaDataList& v_md, Library::Filter filter) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getAllTracksByAlbum(albumIds, v_md, filter, -1);
}

void LocalLibrary::getAllTracksBySearchstring(Library::Filter filter, MetaDataList& v_md) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getAllTracksBySearchString(filter, v_md);
}

void LocalLibrary::getAllTracksByPath(const QStringList& paths, MetaDataList& v_md) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getAllTracksByPaths(paths, v_md);
}

void LocalLibrary::getTrackById(TrackID trackId, MetaData& md) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	MetaData md_tmp = lib_db->getTrackById(trackId);
	if(md_tmp.libraryId() == m->libraryId) {
		md = md_tmp;
	}

	else {
		md = MetaData();
	}
}

void LocalLibrary::getAlbumById(AlbumId albumId, Album& album) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getAlbumByID(albumId, album);
}

void LocalLibrary::getArtistById(ArtistId artistId, Artist& artist) const
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->getArtistByID(artistId, artist);
}

void LocalLibrary::initReloadThread()
{
	if(m->reloadThread){
		return;
	}

	m->reloadThread = new Library::ReloadThread(this);

	connect(m->reloadThread, &Library::ReloadThread::sigReloadingLibrary,
			this, &LocalLibrary::sigReloadingLibrary);

	connect(m->reloadThread, &Library::ReloadThread::sigNewBlockSaved,
			this, &LocalLibrary::reloadThreadNewBlock);

	connect(m->reloadThread, &Library::ReloadThread::finished,
			this, &LocalLibrary::reloadThreadFinished);
}

void LocalLibrary::deleteTracks(const MetaDataList &v_md, Library::TrackDeletionMode mode)
{
	auto* lib_db = DB::Connector::instance()->libraryDatabase(m->libraryId, 0);
	lib_db->deleteTracks(v_md);

	AbstractLibrary::deleteTracks(v_md, mode);
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
	if(files.isEmpty()){
		return;
	}

	if(!m->libraryImporter)
	{
		m->libraryImporter = new Library::Importer(this);
		connect(m->libraryImporter, &Library::Importer::sigStatusChanged, this, &LocalLibrary::importStatusChanged);
	}

	m->libraryImporter->importFiles(files, targetDirectory);

	emit sigImportDialogRequested(targetDirectory);
}

bool LocalLibrary::setLibraryPath(const QString& library_path)
{
	Library::Manager* manager = Library::Manager::instance();
	return manager->changeLibraryPath(m->libraryId, library_path);
}

bool LocalLibrary::setLibraryName(const QString& library_name)
{
	Library::Manager* manager = Library::Manager::instance();
	return manager->renameLibrary(this->id(), library_name);
}

QString LocalLibrary::name() const
{
	Library::Manager* manager = Library::Manager::instance();
	Library::Info info = manager->libraryInfo(this->id());
	return info.name();
}

QString LocalLibrary::path() const
{
	Library::Manager* manager = Library::Manager::instance();
	Library::Info info = manager->libraryInfo(this->id());
	return info.path();
}

LibraryId LocalLibrary::id() const
{
	return m->libraryId;
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
