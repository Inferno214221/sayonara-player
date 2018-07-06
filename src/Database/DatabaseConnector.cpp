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

#include "Database/SayonaraQuery.h"
#include "Database/DatabaseConnector.h"
#include "Database/LibraryDatabase.h"
#include "Database/LocalLibraryDatabase.h"
#include "Database/DatabaseBookmarks.h"
#include "Database/DatabasePlaylist.h"
#include "Database/DatabasePodcasts.h"
#include "Database/DatabaseStreams.h"
#include "Database/DatabaseSettings.h"
#include "Database/DatabaseVisStyles.h"

#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

#include <QFileInfo>
#include <QDateTime>
#include <QTime>

#include <tuple>
#include <algorithm>

using DB::Connector;

struct Connector::Private
{
	DB::Bookmarks*			bookmark_connector=nullptr;
	DB::Playlist*			playlist_connector=nullptr;
	DB::Podcasts*			podcast_connector=nullptr;
	DB::Streams*			stream_connector=nullptr;
	DB::VisualStyles*		visual_style_connector=nullptr;
	DB::Settings*			settings_connector=nullptr;
	DB::Library*			library_connector=nullptr;

	QList<LibraryDatabase*> library_dbs;
	LocalLibraryDatabase*	generic_library_database=nullptr;

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

		if(library_connector){
			delete library_connector; library_connector = nullptr;
		}
	}
};

Connector::Connector() :
	DB::Base(0, "", QString("player.db"), nullptr)
{
	m = Pimpl::make<Private>();
	apply_fixes();

	m->generic_library_database = new DB::LocalLibraryDatabase(-1);
	m->library_dbs << m->generic_library_database;
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
		Query q(db());
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

		Query q(db());
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

bool Connector::apply_fixes()
{
	QString str_version;
	int version;
	bool success;
	const int LatestVersion = 17;

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

	if(version < 3) {
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
			Query q(db());
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
			Query q(db());
			Query q_index(db());
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

		Query q(db());
		q.prepare(querytext);

		if(q.exec()){
			settings_connector()->store_setting("version", 12);
		}
	}

	if(version < 13){
		bool success = check_and_insert_column("tracks", "albumArtistID", "integer", "-1");

		Query q(db());
		q.prepare("UPDATE tracks SET albumArtistID=artistID;");
		success = success && q.exec();

		if(success){
			settings_connector()->store_setting("version", 13);
		}
	}

	if(version < 14){
		bool success=check_and_insert_column("tracks", "libraryID", "integer", "0");
		Query q(db());
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
			LibraryDatabase* lib_db = new LocalLibraryDatabase(-1);
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

	return true;
}


void Connector::clean_up()
{
	Query q(db());
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
	for(LibraryDatabase* db : ::Util::AsConst(m->library_dbs))
	{
		if( (db->library_id() == library_id) &&
			(db->db_id() == db_id))
		{
			return db;
		}
	}

	if(db_id == 0){
		sp_log(Log::Warning, this) << "Could not find Library:"
								" DB ID = " << (int) db_id
							 << " LibraryID = " << (int) library_id;
	}

	return m->generic_library_database;
}


DB::LibraryDatabase *Connector::find_library_db(LibraryId library_id) const
{
	for(DB::LibraryDatabase* db : ::Util::AsConst(m->library_dbs))
	{
		if( (db->library_id() == library_id)){
			return db;
		}
	}

	return nullptr;
}

DB::LibraryDatabase* Connector::register_library_db(LibraryId library_id)
{
	DB::LibraryDatabase* lib_db = find_library_db(library_id);

	if(!lib_db) {
		lib_db = new DB::LocalLibraryDatabase(library_id);
		m->library_dbs << lib_db;
	}

	return lib_db;
}


DB::Playlist* Connector::playlist_connector()
{
	if(!m->playlist_connector){
		m->playlist_connector = new DB::Playlist(this->db(), this->db_id());
	}

	return m->playlist_connector;
}


DB::Bookmarks* Connector::bookmark_connector()
{
	if(!m->bookmark_connector){
		m->bookmark_connector = new DB::Bookmarks(this->db(), this->db_id());
	}

	return m->bookmark_connector;
}

DB::Streams* Connector::stream_connector()
{
	if(!m->stream_connector){
		m->stream_connector = new DB::Streams(this->db(), this->db_id());
	}

	return m->stream_connector;
}

DB::Podcasts* Connector::podcast_connector()
{
	if(!m->podcast_connector){
		m->podcast_connector = new DB::Podcasts(this->db(), this->db_id());
	}

	return m->podcast_connector;
}

DB::VisualStyles* Connector::visual_style_connector()
{
	if(!m->visual_style_connector){
		m->visual_style_connector = new DB::VisualStyles(this->db(), this->db_id());
	}

	return m->visual_style_connector;
}

DB::Settings* Connector::settings_connector()
{
	if(!m->settings_connector){
		m->settings_connector = new DB::Settings(this->db(), this->db_id());
	}

	return m->settings_connector;
}

DB::Library* Connector::library_connector()
{
	if(!m->library_connector){
		m->library_connector = new DB::Library(this->db(), this->db_id());
	}

	return m->library_connector;
}



