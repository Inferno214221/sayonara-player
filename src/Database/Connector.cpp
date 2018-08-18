/* DatabaseConnector.cpp */

/* Copyright (C) 2011-2017 Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General License for more details.

 * You should have received a copy of the GNU General License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Database/Connector.h"
#include "Database/Query.h"
#include "Database/LibraryDatabase.h"
#include "Database/Bookmarks.h"
#include "Database/Playlist.h"
#include "Database/Podcasts.h"
#include "Database/Streams.h"
#include "Database/Settings.h"
#include "Database/Shortcuts.h"
#include "Database/VisStyles.h"

#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/RawShortcutMap.h"

#include <QFileInfo>
#include <QDateTime>
#include <QTime>

#include <tuple>
#include <algorithm>

using DB::Connector;
using DB::LibraryDatabase;

using LibDbIterator=DB::LibraryDatabases::Iterator;

struct Connector::Private
{
	QString					connection_name;

	DB::Bookmarks*			bookmark_connector=nullptr;
	DB::Playlist*			playlist_connector=nullptr;
	DB::Podcasts*			podcast_connector=nullptr;
	DB::Streams*			stream_connector=nullptr;
	DB::VisualStyles*		visual_style_connector=nullptr;
	DB::Settings*			settings_connector=nullptr;
	DB::Shortcuts*			shortcut_connector=nullptr;
	DB::Library*			library_connector=nullptr;

	QList<LibraryDatabase*> library_dbs;
	LibraryDatabase*		generic_library_database=nullptr;

	Private() {}
	~Private()
	{
		if(bookmark_connector){
			delete bookmark_connector; bookmark_connector = nullptr;
		}

		if(podcast_connector){
			delete podcast_connector; podcast_connector = nullptr;
		}

		if(stream_connector){
			delete stream_connector; stream_connector = nullptr;
		}

		if(visual_style_connector){
			delete visual_style_connector; visual_style_connector = nullptr;
		}

		if(settings_connector){
			delete settings_connector; settings_connector = nullptr;
		}

		if(shortcut_connector){
			delete shortcut_connector; shortcut_connector = nullptr;
		}

		if(library_connector){
			delete library_connector; library_connector = nullptr;
		}
	}
};

Connector::Connector() :
	DB::Base(0, "player.db", nullptr)
{
	m = Pimpl::make<Private>();
	m->generic_library_database = new LibraryDatabase(connection_name(), db_id(), -1);
	m->library_dbs << m->generic_library_database;

	apply_fixes();
}

Connector::~Connector() {}

bool Connector::updateAlbumCissearchFix()
{
#ifdef DEBUG_DB
	sp_log(Log::Debug, this) << Q_FUNC_INFO;
#endif

	AlbumList albums;

	LibraryDatabase* lib_db = library_db(-1, 0);
	lib_db->getAllAlbums(albums);

	for(const Album& album : albums)
	{
		QString str = "UPDATE albums SET cissearch=:cissearch WHERE albumID=:id;";
		Query q(this);
		q.prepare(str);
		q.bindValue(":cissearch",	Util::cvt_not_null(album.name().toLower()));
		q.bindValue(":id",			album.id);

		if(!q.exec()){
			q.show_error("Cannot update album cissearch");
		}
	}

	return true;
}


bool Connector::updateArtistCissearchFix()
{
	ArtistList artists;
	LibraryDatabase* lib_db = library_db(-1, 0);
	lib_db->getAllArtists(artists);
	for(const Artist& artist : artists)
	{
		QString str =
				"UPDATE artists SET cissearch=:cissearch WHERE artistID=:id;";

		Query q(this);
		q.prepare(str);
		q.bindValue(":cissearch",	Util::cvt_not_null(artist.name().toLower()));
		q.bindValue(":id",			artist.id);

		if(!q.exec()){
			q.show_error("Cannot update artist cissearch");
		}
	}

	return true;
}

bool Connector::updateTrackCissearchFix()
{
	MetaDataList v_md;
	LibraryDatabase* lib_db = library_db(-1, 0);
	lib_db->getAllTracks(v_md);
	for(const MetaData& md : v_md) {
		lib_db->updateTrack(md);
	}

	return true;
}

bool Connector::updateLostArtists()
{
	LibraryDatabase* lib_db = library_db(-1, 0);
	if(!lib_db){
		sp_log(Log::Error, this) << "Cannot find Library";
		return false;
	}

	ArtistId id = lib_db->insertArtistIntoDatabase(QString());

	const QStringList queries {
		QString("UPDATE tracks SET artistID=:artistID WHERE artistID IN (SELECT artistID FROM artists WHERE name IS NULL);"),
		QString("UPDATE tracks SET artistID=:artistID WHERE artistID NOT IN (SELECT artistID FROM artists);"),
		QString("UPDATE tracks SET albumArtistID=:artistID WHERE albumArtistID IN (SELECT artistID FROM artists WHERE name IS NULL);"),
		QString("UPDATE tracks SET albumArtistID=:artistID WHERE albumArtistID NOT IN (SELECT artistID FROM artists);"),
		QString("DELETE FROM artists WHERE name IS NULL;")
	};

	this->transaction();
	for(const QString& query : queries)
	{
		DB::Query q(this);
		q.prepare(query);
		q.bindValue(":artistID", id);
		bool success = q.exec();
		if(!success){
			this->rollback();
			return false;
		}
	}

	this->commit();
	return true;
}

bool Connector::updateLostAlbums()
{
	LibraryDatabase* lib_db = library_db(-1, 0);
	if(!lib_db){
		sp_log(Log::Error, this) << "Cannot find Library";
		return false;
	}

	AlbumId id = lib_db->insertAlbumIntoDatabase(QString());

	const QStringList queries {
		QString("UPDATE tracks SET albumID=:albumID WHERE albumID IN (SELECT albumID FROM albums WHERE name IS NULL);"),
		QString("UPDATE tracks SET albumID=:albumID WHERE albumID NOT IN (SELECT albumID FROM albums);"),
		QString("DELETE FROM artists WHERE name IS NULL;")
	};

	this->transaction();
	for(const QString& query : queries)
	{
		DB::Query q(this);
		q.prepare(query);
		q.bindValue(":albumID", id);
		bool success = q.exec();
		if(!success){
			this->rollback();
			return false;
		}
	}

	this->commit();
	return true;
}

bool Connector::apply_fixes()
{
	QString str_version;
	int version;
	bool success;
	const int LatestVersion = 19;

	success = settings_connector()->load_setting("version", str_version);
	version = str_version.toInt(&success);
	sp_log(Log::Info, this)
			<< "Database Version:  " << version << ". "
			<< "Latest Version: " << LatestVersion;

	if(version == LatestVersion) {
		sp_log(Log::Info, this) << "No need to update db";
		return true;
	}

	else if(!success){
		 sp_log(Log::Warning, this) << "Cannot get database version";
	}

	sp_log(Log::Info, this) << "Apply fixes";

	if(version < 1)
	{
		check_and_insert_column("playlisttotracks", "position", "INTEGER");
		check_and_insert_column("playlisttotracks", "filepath", "VARCHAR(512)");
		check_and_insert_column("tracks", "genre", "VARCHAR(1024)");

		QString create_savedstreams = QString("CREATE TABLE savedstreams ") +
				"( " +
				"	name VARCHAR(255) PRIMARY KEY, " +
				"	url VARCHAR(255) " +
				");";

		check_and_create_table("savedstreams", create_savedstreams);


		QString create_savedpodcasts = QString("CREATE TABLE savedpodcasts ") +
				"( " +
				"	name VARCHAR(255) PRIMARY KEY, " +
				"	url VARCHAR(255) " +
				");";

		check_and_create_table("savedpodcasts", create_savedpodcasts);
	}

	if(version < 3)
	{
		db().transaction();

		bool success = true;
		success &= check_and_insert_column("tracks", "cissearch", "VARCHAR(512)");
		success &= check_and_insert_column("albums", "cissearch", "VARCHAR(512)");
		success &= check_and_insert_column("artists", "cissearch", "VARCHAR(512)");

		Q_UNUSED(success)

		updateAlbumCissearchFix();
		updateArtistCissearchFix();
		updateTrackCissearchFix();

		db().commit();
	}


	if(version == 3) {
		check_and_drop_table("VisualStyles");
	}

	if(version < 4) {
		QString create_vis_styles = QString("CREATE TABLE VisualStyles ") +
				"( " +
				"  name VARCHAR(255) PRIMARY KEY, " +
				"  col1 VARCHAR(20), "
				"  col2 VARCHAR(20), "
				"  col3 VARCHAR(20), "
				"  col4 VARCHAR(20), "
				"  nBinsSpectrum INTEGER, "
				"  rectHeightSpectrum INTEGER, "
				"  fadingStepsSpectrum INTEGER, "
				"  horSpacingSpectrum INTEGER, "
				"  vertSpacingSpectrum INTEGER, "
				"  rectWidthLevel INTEGER, "
				"  rectHeightLevel INTEGER, "
				"  horSpacingLevel INTEGER, "
				"  verSpacingLevel INTEGER, "
				"  fadingStepsLevel INTEGER "
				");";

		bool success = check_and_create_table("VisualStyles", create_vis_styles);
		if(success) settings_connector()->store_setting("version", 4);
	}

	if(version < 5) {
		bool success = check_and_insert_column("tracks", "rating", "integer");
		if(success) settings_connector()->store_setting("version", 5);
	}

	if(version < 6) {
		QString create_savedbookmarks = QString("CREATE TABLE savedbookmarks ") +
				"( " +
				"	trackid INTEGER, " +
				"	name VARCHAR(255), " +
				"	timeidx INTEGER, " +
				"   PRIMARY KEY (trackid, timeidx), " +
				"   FOREIGN KEY (trackid) REFERENCES tracks(trackid) " +
				");";

		bool success = check_and_create_table("savedbookmarks", create_savedbookmarks);
		if(success) settings_connector()->store_setting("version", 6);
	}

	if(version < 7) {
		bool success = check_and_insert_column("albums", "rating", "integer");
		if(success) settings_connector()->store_setting("version", 7);
	}

	if(version < 9) {
		bool success = check_and_insert_column("playlists", "temporary", "integer");

		if(success) {
			Query q(this);
			QString querytext = "UPDATE playlists SET temporary=0;";
			q.prepare(querytext);
			if(q.exec()){
				settings_connector()->store_setting("version", 9);
			};
		}
	}

	if(version < 10){
		bool success = check_and_insert_column("playlisttotracks", "db_id", "integer");
		if(success) {
			Query q(this);
			Query q_index(this);
			QString querytext = "UPDATE playlisttotracks SET db_id = (CASE WHEN trackid > 0 THEN 0 ELSE -1 END)";
			QString index_query = "CREATE INDEX album_search ON albums(cissearch, albumID);"
					"CREATE INDEX artist_search ON artists(cissearch, artistID);"
					"CREATE INDEX track_search ON tracks(cissearch, trackID);";

			q.prepare(querytext);
			q_index.prepare(index_query);

			if(q.exec()){
				settings_connector()->store_setting("version", 10);
			};

			q_index.exec();
		}
	}

	if(version < 11)
	{
		// look in UpdateDatesThread
	}

	if(version < 12){
		QString querytext =
				"CREATE VIEW album_info_view AS "
				"SELECT "
				"	albums.albumID as albumID, "
				"	albums.name as name, "
				"	albums.cissearch as cissearch, "
				"	albums.rating as rating, "
				"	COUNT(artists.artistID) as artistCount, "
				"	COUNT(tracks.trackID) as trackCount, "
				"	CASE WHEN COUNT(DISTINCT artists.artistID) > 1 "
				"	THEN 1 "
				"	ELSE 0 "
				"	END as Sampler "
				"FROM albums, artists, tracks "
				"WHERE albums.albumID = tracks.albumID "
				"AND artists.artistID = tracks.artistID "
				"GROUP BY albums.albumID, albums.name";
			;

		Query q(this);
		q.prepare(querytext);

		if(q.exec()){
			settings_connector()->store_setting("version", 12);
		}
	}

	if(version < 13){
		bool success = check_and_insert_column("tracks", "albumArtistID", "integer", "-1");

		Query q(this);
		q.prepare("UPDATE tracks SET albumArtistID=artistID;");
		success = success && q.exec();

		if(success){
			settings_connector()->store_setting("version", 13);
		}
	}

	if(version < 14){
		bool success=check_and_insert_column("tracks", "libraryID", "integer", "0");
		Query q(this);
		q.prepare("UPDATE tracks SET libraryID=0;");
		success = success && q.exec();

		if(success){
			settings_connector()->store_setting("version", 14);
		}
	}

	if(version < 15)
	{
		QString create_string =
			"CREATE TABLE Libraries "
			"( "
			"  libraryID INTEGER NOT NULL, "
			"  libraryName VARCHAR(128) NOT NULL, "
			"  libraryPath VARCHAR(512) NOT NULL, "
			"  libraryIndex INTEGER NOT NULL,"
			"  PRIMARY KEY (libraryID, libraryPath) "
			"); ";

		bool success=check_and_create_table("Libraries", create_string);
		if(success)
		{
			settings_connector()->store_setting("version", 15);
		}
	}

	if(version < 16)
	{
		bool success = check_and_insert_column("tracks", "fileCissearch", "VARCHAR(256)");

		if(success)
		{
			settings_connector()->store_setting("version", 16);

			MetaDataList v_md;
			LibraryDatabase* lib_db = new DB::LibraryDatabase(connection_name(), db_id(), -1);
			lib_db->getAllTracks(v_md);
			this->transaction();
			for(const MetaData& md : v_md) {
				lib_db->updateTrack(md);
			}
			this->commit();

			delete lib_db;
		}
	}

	if(version < 17)
	{
		bool success = check_and_insert_column("tracks", "comment", "VARCHAR(1024)");

		if(success)
		{
			settings_connector()->store_setting("version", 17);
		}
	}

	if(version < 18)
	{
		if(updateLostArtists() && updateLostAlbums())
		{
			settings_connector()->store_setting("version", 18);
		}
	}

	if(version < 19)
	{
		QString create_string =
			"CREATE TABLE Shortcuts "
			"( "
			"  id INTEGER NOT NULL PRIMARY KEY, "
			"  identifier VARCHAR(32) NOT NULL, "
			"  shortcut VARCHAR(32) NOT NULL "
			"); ";

		bool success = check_and_create_table("Shortcuts", create_string);
		if(success)
		{
			QString raw;
			settings_connector()->load_setting("shortcuts", raw);

			RawShortcutMap rsm = RawShortcutMap::fromString(raw);
			for(const QString& key : rsm.keys())
			{
				this->shortcut_connector()->setShortcuts(key, rsm.value(key));
			}

			settings_connector()->store_setting("shortcuts", "<deprecated>");
			settings_connector()->store_setting("version", 19);
		}
	}

	return true;
}


void Connector::clean_up()
{
	Query q(this);
	QString querytext = "VACUUM;";
	q.prepare(querytext);
	q.exec();
}

DB::LibraryDatabases Connector::library_dbs() const
{
	return m->library_dbs;
}


DB::LibraryDatabase* Connector::library_db(LibraryId library_id, DbId db_id)
{
	LibDbIterator it = Util::find(m->library_dbs, [=](DB::LibraryDatabase* db){
		return (db->library_id() == library_id && db->db_id() == db_id);
	});

	if(it == m->library_dbs.end())
	{
		sp_log(Log::Warning, this) << "Could not find Library:"
								" DB ID = " << (int) db_id
							 << " LibraryID = " << (int) library_id;

		return m->generic_library_database;
	}

	return *it;
}


DB::LibraryDatabase* Connector::register_library_db(LibraryId library_id)
{
	DB::LibraryDatabase* lib_db = nullptr;
	LibDbIterator it = Util::find(m->library_dbs, [=](DB::LibraryDatabase* db){
		return (db->library_id() == library_id);
	});

	if(it == m->library_dbs.end())
	{
		lib_db = new DB::LibraryDatabase(this->connection_name(), this->db_id(), library_id);
		m->library_dbs << lib_db;
	}

	else
	{
		lib_db = *it;
	}

	return lib_db;
}

void Connector::delete_library_db(LibraryId library_id)
{
	LibDbIterator it = Util::find(m->library_dbs, [=](DB::LibraryDatabase* db){
		return (db->library_id() == library_id);
	});

	if(it != m->library_dbs.end())
	{
		LibraryDatabase* db = *it;
		db->deleteAllTracks(true);
		m->library_dbs.removeAll(db);

		delete db; db = nullptr;
	}
}


DB::Playlist* Connector::playlist_connector()
{
	if(!m->playlist_connector){
		m->playlist_connector = new DB::Playlist(this->connection_name(), this->db_id());
	}

	return m->playlist_connector;
}


DB::Bookmarks* Connector::bookmark_connector()
{
	if(!m->bookmark_connector){
		m->bookmark_connector = new DB::Bookmarks(this->connection_name(), this->db_id());
	}

	return m->bookmark_connector;
}

DB::Streams* Connector::stream_connector()
{
	if(!m->stream_connector){
		m->stream_connector = new DB::Streams(this->connection_name(), this->db_id());
	}

	return m->stream_connector;
}

DB::Podcasts* Connector::podcast_connector()
{
	if(!m->podcast_connector){
		m->podcast_connector = new DB::Podcasts(this->connection_name(), this->db_id());
	}

	return m->podcast_connector;
}

DB::VisualStyles* Connector::visual_style_connector()
{
	if(!m->visual_style_connector){
		m->visual_style_connector = new DB::VisualStyles(this->connection_name(), this->db_id());
	}

	return m->visual_style_connector;
}

DB::Settings* Connector::settings_connector()
{
	if(!m->settings_connector){
		m->settings_connector = new DB::Settings(this->connection_name(), this->db_id());
	}

	return m->settings_connector;
}

DB::Shortcuts*Connector::shortcut_connector()
{
	if(!m->shortcut_connector){
		m->shortcut_connector = new DB::Shortcuts(this->connection_name(), this->db_id());
	}

	return m->shortcut_connector;
}

DB::Library* Connector::library_connector()
{
	if(!m->library_connector){
		m->library_connector = new DB::Library(this->connection_name(), this->db_id());
	}

	return m->library_connector;
}



