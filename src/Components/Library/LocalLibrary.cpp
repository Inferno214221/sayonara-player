/* LocalLibrary.cpp */

/* Copyright (C) 2011-2017 Lucio Carreras
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
#include "LibraryManager.h"

#include "Importer/LibraryImporter.h"
#include "Threads/ReloadThread.h"

#include "Database/DatabaseConnector.h"
#include "Database/LocalLibraryDatabase.h"
#include "Database/DatabaseLibrary.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Tagging/Editor.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"
#include "Utils/globals.h"

#include <utility>
#include <QTime>

struct LocalLibrary::Private
{
	Library::ReloadThread*	reload_thread=nullptr;
	Library::Importer*		library_importer=nullptr;

	DB::Connector*			db=nullptr;
	DB::LibraryDatabase*	library_db=nullptr;

	LibraryId				library_id;

	Private(LibraryId library_id) :
		db(DB::Connector::instance()),
		library_db(db->library_db(library_id, 0)),
		library_id(library_id)
	{}
};

LocalLibrary::LocalLibrary(LibraryId library_id, QObject *parent) :
	AbstractLibrary(parent)
{
	DB::Connector::instance()->register_library_db(library_id);

	m = Pimpl::make<Private>(library_id);

	apply_db_fixes();

	Playlist::Handler* plh = Playlist::Handler::instance();
	connect(plh, &Playlist::Handler::sig_track_deletion_requested,
			this, &LocalLibrary::delete_tracks);

	Set::listen<Set::Lib_SearchMode>(this, &LocalLibrary::_sl_search_mode_changed, false);
}

LocalLibrary::~LocalLibrary() {}

void LocalLibrary::clear_library()
{
	m->library_db->clear();
}

void LocalLibrary::apply_db_fixes() {}


void LocalLibrary::reload_library(bool clear_first, Library::ReloadQuality quality)
{
	if(m->reload_thread && m->reload_thread->is_running()){
		return;
	}

	if(!m->reload_thread){
		init_reload_thread();
	}

	if(clear_first) {
		delete_all_tracks();
	}

	m->reload_thread->set_quality(quality);
	m->reload_thread->set_library(library_id(), library_path());
	m->reload_thread->start();
}


void LocalLibrary::reload_thread_finished()
{
	load();

	emit sig_reloading_library("", -1);
	emit sig_reloading_library_finished();
}

void LocalLibrary::_sl_search_mode_changed()
{
	sp_log(Log::Debug, this) << "Updating cissearch... " << _settings->get<Set::Lib_SearchMode>();

	m->library_db->updateArtistCissearch();
	m->library_db->updateAlbumCissearch();
	m->library_db->updateTrackCissearch();

	sp_log(Log::Debug, this) << "Updating cissearch finished" << _settings->get<Set::Lib_SearchMode>();
}


void LocalLibrary::library_reloading_state_new_block()
{
	::Library::Sortings so = sortorder();
	m->reload_thread->pause();

	m->library_db->getAllAlbums(_albums, so.so_albums);
	m->library_db->getAllArtists(_artists, so.so_artists);
	m->library_db->getAllTracks(_tracks, so.so_tracks);

	emit_stuff();

	m->reload_thread->goon();
}


void LocalLibrary::psl_disc_pressed(int disc)
{
	if( selected_albums().size() != 1 )
	{
		return;
	}

	MetaDataList v_md;

	if(disc < 0)
	{
		m->library_db->getAllTracksByAlbum(selected_albums().first(), _tracks, filter(), sortorder().so_tracks);
	}

	else
	{
		m->library_db->getAllTracksByAlbum(selected_albums().first(), v_md, filter(), sortorder().so_tracks);

		_tracks.clear();

		for(const MetaData& md : v_md)
		{
			if(md.discnumber != disc) {
				continue;
			}

			_tracks << std::move(md);
		}
	}

	_tracks.sort(sortorder().so_tracks);

	emit sig_all_tracks_loaded();
}




void LocalLibrary::get_all_artists(ArtistList& artists)
{
	m->library_db->getAllArtists(artists, Library::SortOrder::NoSorting);
}

void LocalLibrary::get_all_artists_by_searchstring(Library::Filter filter, ArtistList& artists)
{
	m->library_db->getAllArtistsBySearchString(filter, artists, Library::SortOrder::NoSorting);
}


void LocalLibrary::get_all_albums(AlbumList& albums)
{
	m->library_db->getAllAlbums(albums, Library::SortOrder::NoSorting);
}


void LocalLibrary::get_all_albums_by_artist(IdList artist_ids, AlbumList& albums, Library::Filter filter)
{
	m->library_db->getAllAlbumsByArtist(artist_ids, albums, filter, Library::SortOrder::NoSorting);
}


void LocalLibrary::get_all_albums_by_searchstring(Library::Filter filter, AlbumList& albums)
{
	m->library_db->getAllAlbumsBySearchString(filter, albums, Library::SortOrder::NoSorting);
}


void LocalLibrary::get_all_tracks(MetaDataList& v_md)
{
	m->library_db->getAllTracks(v_md, Library::SortOrder::NoSorting);
}


void LocalLibrary::get_all_tracks(const QStringList& paths, MetaDataList& v_md)
{
	m->library_db->getMultipleTracksByPath(paths, v_md);
}


void LocalLibrary::get_all_tracks_by_artist(IdList artist_ids, MetaDataList& v_md, Library::Filter filter)
{
	m->library_db->getAllTracksByArtist(artist_ids, v_md, filter, Library::SortOrder::NoSorting);
}


void LocalLibrary::get_all_tracks_by_album(IdList album_ids, MetaDataList& v_md, Library::Filter filter)
{
	m->library_db->getAllTracksByAlbum(album_ids, v_md, filter, Library::SortOrder::NoSorting);
}


void LocalLibrary::get_all_tracks_by_searchstring(Library::Filter filter, MetaDataList& v_md)
{
	m->library_db->getAllTracksBySearchString(filter, v_md, Library::SortOrder::NoSorting);
}


void LocalLibrary::get_album_by_id(AlbumId album_id, Album& album)
{
	m->library_db->getAlbumByID(album_id, album);
}


void LocalLibrary::get_artist_by_id(ArtistId artist_id, Artist& artist)
{
	m->library_db->getArtistByID(artist_id, artist);
}


void LocalLibrary::update_track(const MetaData& md)
{
	m->library_db->updateTrack(md);
}

void LocalLibrary::update_tracks(const MetaDataList& v_md)
{
	m->library_db->updateTracks(v_md);
}

void LocalLibrary::update_album(const Album& album)
{
	m->library_db->updateAlbum(album);
}


void LocalLibrary::insert_tracks(const MetaDataList &v_md)
{
	m->library_db->store_metadata(v_md);
	AbstractLibrary::insert_tracks(v_md);
}


void LocalLibrary::init_reload_thread()
{
	if(m->reload_thread){
		return;
	}

	m->reload_thread = new Library::ReloadThread(this);

	connect(m->reload_thread, &Library::ReloadThread::sig_reloading_library,
			this, &LocalLibrary::sig_reloading_library);

	connect(m->reload_thread, &Library::ReloadThread::sig_new_block_saved,
			this, &LocalLibrary::library_reloading_state_new_block);

	connect(m->reload_thread, &Library::ReloadThread::finished,
			this, &LocalLibrary::reload_thread_finished);
}


void LocalLibrary::delete_tracks(const MetaDataList &v_md, Library::TrackDeletionMode mode)
{
	m->library_db->deleteTracks(v_md);

	AbstractLibrary::delete_tracks(v_md, mode);
}

void LocalLibrary::refresh_artist() {}

void LocalLibrary::refresh_albums() {}

void LocalLibrary::refresh_tracks() {}

void LocalLibrary::import_files(const QStringList& files)
{
	import_files(files, QString());
}

void LocalLibrary::import_files(const QStringList &files, const QString &target_dir)
{
	if(!m->library_importer){
		m->library_importer = new Library::Importer(this);
	}

	m->library_importer->import_files(files, target_dir);

	emit sig_import_dialog_requested(target_dir);
}


/** BIG TODO
 * What is the library for? Imo, the Library is there
 * for managing the database entries for the tracks
 * So, the library is NOT responsible for changing
 * The ID3 tags on filesystem base. So, these 3
 * methods should be moved somewhere else.
 * You can use the updateTracks method for doing
 * the database part when editing tracks.
 * But I suggest, to introduce a Library/TagEdit
 * interface which you can use to edit tracks. But
 * this is not part of this ticket.
 */

void LocalLibrary::merge_artists(const SP::Set<ArtistId>& artist_ids, ArtistId target_artist)
{
	if(artist_ids.isEmpty()) {
		return;
	}

	if(target_artist < 0){
		sp_log(Log::Warning, this) << "Cannot merge artist: Target artist id < 0";
		return;
	}

	bool show_album_artists = _settings->get<Set::Lib_ShowAlbumArtists>();

	Artist artist;
	bool success = m->library_db->getArtistByID(target_artist, artist);
	if(!success){
		return;
	}

	MetaDataList v_md;

	get_all_tracks_by_artist(artist_ids.toList(), v_md, filter());
	tag_edit()->set_metadata(v_md);

	for(int idx=0; idx<v_md.count(); idx++)
	{
		MetaData md(v_md[idx]);
		if(show_album_artists){
			md.set_album_artist(artist.name(), artist.id);
		}

		else {
			md.artist_id = artist.id;
			md.set_artist(artist.name());
		}

		tag_edit()->update_track(idx, md);
	}

	tag_edit()->commit();
}

void LocalLibrary::merge_albums(const SP::Set<AlbumId>& album_ids, AlbumId target_album)
{
	if(album_ids.isEmpty())	{
		return;
	}

	if(target_album < 0){
		sp_log(Log::Warning, this) << "Cannot merge albums: Target album id < 0";
		return;
	}

	Album album;
	bool success = m->library_db->getAlbumByID(target_album, album, true);
	if(!success) {
		return;
	}

	MetaDataList v_md;
	get_all_tracks_by_album(album_ids.toList(), v_md, filter());

	tag_edit()->set_metadata(v_md);

	for(int idx=0; idx<v_md.count(); idx++)
	{
		MetaData md(v_md[idx]);
		md.album_id = album.id;
		md.set_album(album.name());

		tag_edit()->update_track(idx, md);
	}

	tag_edit()->commit();
}

void LocalLibrary::show_album_artists_changed(bool show_album_artists)
{
	DB::LibraryDatabases dbs = m->db->library_dbs();
	for(DB::LibraryDatabase* lib_db : dbs)
	{
		if(lib_db->db_id() == 0)
		{
			if(show_album_artists)
			{
				lib_db->change_artistid_field(DB::LibraryDatabase::ArtistIDField::AlbumArtistID);
			}

			else
			{
				lib_db->change_artistid_field(DB::LibraryDatabase::ArtistIDField::ArtistID);
			}
		}
	}

	refresh();
}

void LocalLibrary::change_track_rating(int idx, Rating rating)
{
	MetaDataList v_md{ _tracks[idx] };

	AbstractLibrary::change_track_rating(idx, rating);
	MetaData md_new = _tracks[idx];

	tag_edit()->set_metadata(v_md);
	tag_edit()->update_track(0, md_new);
	tag_edit()->commit();
}

bool LocalLibrary::set_library_path(const QString& library_path)
{
	Library::Manager* manager = Library::Manager::instance();
	return manager->change_library_path(m->library_id, library_path);
}

bool LocalLibrary::set_library_name(const QString& library_name)
{
	Library::Manager* manager = Library::Manager::instance();
	return manager->rename_library(this->library_id(), library_name);
}

QString LocalLibrary::library_name() const
{
	Library::Manager* manager = Library::Manager::instance();
	Library::Info info = manager->library_info(this->library_id());
	return info.name();
}

QString LocalLibrary::library_path() const
{
	Library::Manager* manager = Library::Manager::instance();
	Library::Info info = manager->library_info(this->library_id());
	return info.path();
}

LibraryId LocalLibrary::library_id() const
{
	return m->library_id;
}

Library::Importer* LocalLibrary::importer()
{
	if(!m->library_importer){
		m->library_importer = new Library::Importer(this);
	}

	return m->library_importer;
}
