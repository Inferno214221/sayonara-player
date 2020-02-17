/* DatabaseConnector.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
#include "Database/Session.h"
#include "Database/Settings.h"
#include "Database/Shortcuts.h"
#include "Database/VisualStyles.h"
#include "Database/CoverConnector.h"

#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/RawShortcutMap.h"

#include <QFileInfo>
#include <QDateTime>
#include <QTime>

#include <tuple>
#include <algorithm>

using DB::Connector;
using DB::LibraryDatabase;

using LibDbIterator=DB::LibraryDatabases::Iterator;
namespace Algorithm=Util::Algorithm;

struct Connector::Private
{
	QString					connection_name;
	QString					default_sourceDirectory;
	QString					default_targetDirectory;
	QString					default_databseFilename;

	DB::Bookmarks*			bookmark_connector=nullptr;
	DB::Playlist*			playlist_connector=nullptr;
	DB::Podcasts*			podcast_connector=nullptr;
	DB::Streams*			stream_connector=nullptr;
	DB::VisualStyles*		visual_style_connector=nullptr;
	DB::Session*			session_connector=nullptr;
	DB::Settings*			settings_connector=nullptr;
	DB::Shortcuts*			shortcut_connector=nullptr;
	DB::Covers*				cover_connector=nullptr;
	DB::Library*			library_connector=nullptr;

	QList<LibraryDatabase*> library_dbs;
	LibraryDatabase*		generic_library_database=nullptr;

	int						old_db_version;


	Private() : old_db_version(0) {}
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

		if(cover_connector){
			delete cover_connector; cover_connector = nullptr;
		}

		if(library_connector){
			delete library_connector; library_connector = nullptr;
		}

		if(session_connector){
			delete session_connector; session_connector = nullptr;
		}
	}
};

class DatabaseNotCreatedException : public std::exception
{
public:
	const char* what() const noexcept;
};


Connector::Connector(const QString& sourceDirectory, const QString& targetDirectory, const QString& databseFilename) :
	DB::Base(0, sourceDirectory, targetDirectory, databseFilename, nullptr)
{
	m = Pimpl::make<Private>();

	if(!this->isInitialized()){
		throw DatabaseNotCreatedException();
	}

	else
	{
		m->generic_library_database = new LibraryDatabase(connectionName(), databaseId(), -1);
		m->library_dbs << m->generic_library_database;

		applyFixes();
	}
}

Connector::~Connector() = default;

DB::Connector* Connector::instance()
{
	return instance_custom(QString(), QString(), QString());
}

DB::Connector* Connector::instance_custom(QString sourceDirectory, QString targetDirectory, QString databseFilename)
{
	if(sourceDirectory.isEmpty()) {
		sourceDirectory = ":/Database";
	}

	if(targetDirectory.isEmpty()) {
		targetDirectory = Util::sayonaraPath();
	}

	if(databseFilename.isEmpty()) {
		databseFilename = "player.db";
	}

	static Connector connector(sourceDirectory, targetDirectory, databseFilename);
	return &connector;
}


bool Connector::updateAlbumCissearchFix()
{
#ifdef DEBUG_DB
	sp_log(Log::Debug, this) << Q_FUNC_INFO;
#endif

	AlbumList albums;

	LibraryDatabase* lib_db = libraryDatabase(-1, 0);
	lib_db->getAllAlbums(albums, true);

	for(const Album& album : albums)
	{
		QString str = "UPDATE albums SET cissearch=:cissearch WHERE albumID=:id;";
		Query q(this);
		q.prepare(str);
		q.bindValue(":cissearch",	Util::convertNotNull(album.name().toLower()));
		q.bindValue(":id",			album.id());

		if(!q.exec()){
			q.showError("Cannot update album cissearch");
		}
	}

	return true;
}

bool Connector::updateArtistCissearchFix()
{
	ArtistList artists;
	LibraryDatabase* lib_db = libraryDatabase(-1, 0);
	lib_db->getAllArtists(artists, true);
	for(const Artist& artist : artists)
	{
		QString str =
				"UPDATE artists SET cissearch=:cissearch WHERE artistID=:id;";

		Query q(this);
		q.prepare(str);
		q.bindValue(":cissearch",	Util::convertNotNull(artist.name().toLower()));
		q.bindValue(":id",			artist.id());

		if(!q.exec()){
			q.showError("Cannot update artist cissearch");
		}
	}

	return true;
}

bool Connector::updateTrackCissearchFix()
{
	MetaDataList v_md;
	LibraryDatabase* lib_db = libraryDatabase(-1, 0);
	lib_db->getAllTracks(v_md);
	for(const MetaData& md : v_md) {
		lib_db->updateTrack(md);
	}

	return true;
}

bool Connector::updateLostArtists()
{
	LibraryDatabase* lib_db = libraryDatabase(-1, 0);
	if(!lib_db){
		spLog(Log::Error, this) << "Cannot find Library";
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
	LibraryDatabase* lib_db = libraryDatabase(-1, 0);
	if(!lib_db){
		spLog(Log::Error, this) << "Cannot find Library";
		return false;
	}

	AlbumId id = lib_db->insertAlbumIntoDatabase(QString(""));

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

int Connector::oldDatabaseVersion() const
{
	return m->old_db_version;
}

int Connector::highestDatabaseVersion()
{
	return 25;
}

bool Connector::applyFixes()
{
	QString str_version;
	int version;
	bool success;
	const int LatestVersion = highestDatabaseVersion();

	success = settingsConnector()->loadSetting("version", str_version);
	version = str_version.toInt(&success);
	m->old_db_version = version;

	spLog(Log::Info, this)
			<< "Database Version:  " << version << ". "
			<< "Latest Version: " << LatestVersion;

	if(version == LatestVersion) {
		spLog(Log::Info, this) << "No need to update db";
		return true;
	}

	else if(!success){
		 spLog(Log::Warning, this) << "Cannot get database version";
	}

	spLog(Log::Info, this) << "Apply fixes";

	if(version < 1)
	{
		checkAndInsertColumn("playlisttotracks", "position", "INTEGER");
		checkAndInsertColumn("playlisttotracks", "filepath", "VARCHAR(512)");
		checkAndInsertColumn("tracks", "genre", "VARCHAR(1024)");

		QString create_savedstreams = QString("CREATE TABLE savedstreams ") +
				"( " +
				"	name VARCHAR(255) PRIMARY KEY, " +
				"	url VARCHAR(255) " +
				");";

		checkAndCreateTable("savedstreams", create_savedstreams);


		QString create_savedpodcasts = QString("CREATE TABLE savedpodcasts ") +
				"( " +
				"	name VARCHAR(255) PRIMARY KEY, " +
				"	url VARCHAR(255) " +
				");";

		checkAndCreateTable("savedpodcasts", create_savedpodcasts);
	}

	if(version < 3)
	{
		db().transaction();

		bool success = true;
		success &= checkAndInsertColumn("tracks", "cissearch", "VARCHAR(512)");
		success &= checkAndInsertColumn("albums", "cissearch", "VARCHAR(512)");
		success &= checkAndInsertColumn("artists", "cissearch", "VARCHAR(512)");

		Q_UNUSED(success)

		updateAlbumCissearchFix();
		updateArtistCissearchFix();
		updateTrackCissearchFix();

		db().commit();
	}


	if(version == 3) {
		checkAndDropTable("VisualStyles");
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

		bool success = checkAndCreateTable("VisualStyles", create_vis_styles);
		if(success) settingsConnector()->storeSetting("version", 4);
	}

	if(version < 5) {
		bool success = checkAndInsertColumn("tracks", "rating", "integer");
		if(success) settingsConnector()->storeSetting("version", 5);
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

		bool success = checkAndCreateTable("savedbookmarks", create_savedbookmarks);
		if(success) settingsConnector()->storeSetting("version", 6);
	}

	if(version < 7) {
		bool success = checkAndInsertColumn("albums", "rating", "integer");
		if(success) settingsConnector()->storeSetting("version", 7);
	}

	if(version < 9) {
		bool success = checkAndInsertColumn("playlists", "temporary", "integer");

		if(success) {
			Query q(this);
			QString querytext = "UPDATE playlists SET temporary=0;";
			q.prepare(querytext);
			if(q.exec()){
				settingsConnector()->storeSetting("version", 9);
			};
		}
	}

	if(version < 10){
		bool success = checkAndInsertColumn("playlisttotracks", "db_id", "integer");
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
				settingsConnector()->storeSetting("version", 10);
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
			settingsConnector()->storeSetting("version", 12);
		}
	}

	if(version < 13){
		bool success = checkAndInsertColumn("tracks", "albumArtistID", "integer", "-1");

		Query q(this);
		q.prepare("UPDATE tracks SET albumArtistID=artistID;");
		success = success && q.exec();

		if(success){
			settingsConnector()->storeSetting("version", 13);
		}
	}

	if(version < 14){
		bool success=checkAndInsertColumn("tracks", "libraryID", "integer", "0");
		Query q(this);
		q.prepare("UPDATE tracks SET libraryID=0;");
		success = success && q.exec();

		if(success){
			settingsConnector()->storeSetting("version", 14);
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

		bool success=checkAndCreateTable("Libraries", create_string);
		if(success)
		{
			settingsConnector()->storeSetting("version", 15);
		}
	}

	if(version < 16)
	{
		bool success = checkAndInsertColumn("tracks", "fileCissearch", "VARCHAR(256)");

		if(success)
		{
			settingsConnector()->storeSetting("version", 16);

			MetaDataList v_md;
			LibraryDatabase* lib_db = new DB::LibraryDatabase(connectionName(), databaseId(), -1);
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
		bool success = checkAndInsertColumn("tracks", "comment", "VARCHAR(1024)");

		if(success)
		{
			settingsConnector()->storeSetting("version", 17);
		}
	}

	if(version < 18)
	{
		if(updateLostArtists() && updateLostAlbums())
		{
			settingsConnector()->storeSetting("version", 18);
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

		bool success = checkAndCreateTable("Shortcuts", create_string);
		if(success)
		{
			QString raw;
			settingsConnector()->loadSetting("shortcuts", raw);

			RawShortcutMap rsm = RawShortcutMap::fromString(raw);
			for(const QString& key : rsm.keys())
			{
				this->shortcutConnector()->setShortcuts(key, rsm.value(key));
			}

			settingsConnector()->storeSetting("shortcuts", "<deprecated>");
			settingsConnector()->storeSetting("version", 19);
		}
	}

	if(version < 20)
	{
		checkAndDropTable("Covers");

		bool success;
		{
			QString create_string =
				"CREATE TABLE Covers "
				"("
				"  coverId INTEGER PRIMARY KEY,"
				"  hash VARCHAR(64),"
				"  coverKey VARCHAR(128),"
				"  data BLOB "
				");";

			success = checkAndCreateTable("Covers", create_string);
		}

		{
			QString create_string =
				"CREATE TABLE TrackCoverMap "
				"("
				"  metadataId INTEGER,"
				"  coverId INTEGER,"
				"  PRIMARY KEY(metadataId, coverId),"
				"  FOREIGN KEY(metadataId) REFERENCES Tracks(trackId) ON DELETE CASCADE,"
				"  FOREIGN KEY(coverId) REFERENCES Covers(coverId) ON DELETE CASCADE"
				");";

			success &= checkAndCreateTable("TrackCoverMap", create_string);
		}

		{
			QString create_string =
				"CREATE TABLE AlbumCoverMap "
				"("
				"  albumId INTEGER,"
				"  coverId INTEGER,"
				"  PRIMARY KEY(albumId, coverId),"
				"  FOREIGN KEY(albumId) REFERENCES Albums(albumId) ON DELETE CASCADE,"
				"  FOREIGN KEY(coverId) REFERENCES Covers(coverId) ON DELETE CASCADE"
				");";

			success &= checkAndCreateTable("AlbumCoverMap", create_string);
		}

		if(success)
		{
			settingsConnector()->storeSetting("version", 20);
		}
	}

	if(version < 21)
	{
		checkAndDropTable("Statistics");
		checkAndDropTable("Lyrics");
		checkAndDropTable("Genres");

		settingsConnector()->storeSetting("version", 21);
	}

	if(version < 22)
	{
		LibraryDatabase* lib_db = libraryDatabase(-1, 0);

		QMap<QString, AlbumId> albums;
		QMap<QString, ArtistId> artists;

		MetaDataList tracks;
		lib_db->getAllTracks(tracks);

		for(auto it=tracks.begin(); it != tracks.end(); it++)
		{
			albums[it->album()] = it->albumId();
			artists[it->artist()] = it->artistId();
			artists[it->albumArtist()] = it->albumArtistId();
		}

		for(auto it=tracks.begin(); it != tracks.end(); it++)
		{
			AlbumId correct_albumId = albums[it->album()];
			ArtistId correct_artistId = artists[it->artist()];
			ArtistId correct_album_artistId = artists[it->albumArtist()];
			this->transaction();
			if(	(it->albumId() != correct_albumId) ||
				(it->artistId() != correct_artistId) ||
				(it->albumArtistId() != correct_album_artistId))
			{
				spLog(Log::Info, this) << "Move track " << it->filepath() << "from album " << it->albumId() << " to " << correct_albumId;

				it->setAlbumId(correct_albumId);
				it->setArtistId(correct_artistId);
				it->setAlbumArtistId(correct_album_artistId);

				lib_db->updateTrack(*it);
			}
			this->commit();
		}

		{
			QString query("DELETE FROM albums WHERE albums.albumID NOT IN (SELECT albumId from tracks);");
			Query q(this);
			q.prepare(query);
			q.exec();
		}

		{
			QString query("DELETE FROM artists WHERE artists.artistID NOT IN (SELECT artistId from tracks UNION SELECT albumArtistId FROM tracks);");
			Query q(this);
			q.prepare(query);
			q.exec();
		}

		settingsConnector()->storeSetting("version", 22);
	}

	if(version < 23)
	{
		{
			checkAndCreateTable(
				"Sessions",
				"CREATE TABLE IF NOT EXISTS Sessions "
				"("
				"    sessionID INTEGER, "
				"    date INTEGER, "
				"    trackID INTEGER DEFAULT -1 REFERENCES Tracks(trackID) ON DELETE SET DEFAULT, "
				"    title VARCHAR(128), "
				"    artist VARCHAR(128), "
				"    album VARCHAR(128)"
				");"
			);
		}

		{
			const QString query
			(
				"INSERT INTO Sessions "
				"SELECT session.date, session.date, session.trackID, tracks.title, artists.name, albums.name "
				"FROM Session session, Tracks tracks, Artists artists, Albums albums "
				"WHERE tracks.artistID = artists.artistID AND tracks.albumID = albums.albumID AND Session.trackID = tracks.trackID "
				";"
			);

			Query q(this);
			q.prepare(query);
			q.exec();
		}

		{
			checkAndDropTable("Session");
		}

		settingsConnector()->storeSetting("version", 23);
	}

	if(version < 24)
	{

		{
			const QString query
			(
				"UPDATE Sessions SET sessionID = (sessionID + 20000000000000) WHERE sessionID < 1000000000000;"
			);

			Query q(this);
			q.prepare(query);
			q.exec();

		}
		{
			const QString query
			(
				"UPDATE Sessions SET date = (date + 20000000000000) WHERE date < 1000000000000;"
			);

			Query q(this);
			q.prepare(query);
			q.exec();
		}

		settingsConnector()->storeSetting("version", 24);
	}

	if(version < 25)
	{
		checkAndInsertColumn("savedpodcasts", "reversed", "INTEGER", "0");
		settingsConnector()->storeSetting("version", 25);
	}

	return true;
}

DB::LibraryDatabases Connector::libraryDatabases() const
{
	return m->library_dbs;
}

DB::LibraryDatabase* Connector::libraryDatabase(LibraryId libraryId, DbId databaseId)
{
	LibDbIterator it = Algorithm::find(m->library_dbs, [=](DB::LibraryDatabase* db){
		return (db->libraryId() == libraryId && db->databaseId() == databaseId);
	});

	if(it == m->library_dbs.end())
	{
		spLog(Log::Warning, this) << "Could not find Library:"
								" DB ID = " << int(databaseId)
							 << " LibraryID = " << int(libraryId);

		return m->generic_library_database;
	}

	return *it;
}


DB::LibraryDatabase* Connector::registerLibraryDatabase(LibraryId libraryId)
{
	DB::LibraryDatabase* lib_db = nullptr;
	LibDbIterator it = Algorithm::find(m->library_dbs, [=](DB::LibraryDatabase* db){
		return (db->libraryId() == libraryId);
	});

	if(it == m->library_dbs.end())
	{
		lib_db = new DB::LibraryDatabase(this->connectionName(), this->databaseId(), libraryId);
		m->library_dbs << lib_db;
	}

	else
	{
		lib_db = *it;
	}

	return lib_db;
}

void Connector::deleteLibraryDatabase(LibraryId libraryId)
{
	LibDbIterator it = Algorithm::find(m->library_dbs, [=](DB::LibraryDatabase* db){
		return (db->libraryId() == libraryId);
	});

	if(it != m->library_dbs.end())
	{
		LibraryDatabase* db = *it;
		db->deleteAllTracks(true);
		m->library_dbs.removeAll(db);

		delete db; db = nullptr;
	}
}


DB::Playlist* Connector::playlistConnector()
{
	if(!m->playlist_connector){
		m->playlist_connector = new DB::Playlist(this->connectionName(), this->databaseId());
	}

	return m->playlist_connector;
}


DB::Bookmarks* Connector::bookmarkConnector()
{
	if(!m->bookmark_connector){
		m->bookmark_connector = new DB::Bookmarks(this->connectionName(), this->databaseId());
	}

	return m->bookmark_connector;
}

DB::Streams* Connector::streamConnector()
{
	if(!m->stream_connector){
		m->stream_connector = new DB::Streams(this->connectionName(), this->databaseId());
	}

	return m->stream_connector;
}

DB::Podcasts* Connector::podcastConnector()
{
	if(!m->podcast_connector){
		m->podcast_connector = new DB::Podcasts(this->connectionName(), this->databaseId());
	}

	return m->podcast_connector;
}

DB::VisualStyles* Connector::visualStyleConnector()
{
	if(!m->visual_style_connector){
		m->visual_style_connector = new DB::VisualStyles(this->connectionName(), this->databaseId());
	}

	return m->visual_style_connector;
}

DB::Settings* Connector::settingsConnector()
{
	if(!m->settings_connector){
		m->settings_connector = new DB::Settings(this->connectionName(), this->databaseId());
	}

	return m->settings_connector;
}

DB::Shortcuts*Connector::shortcutConnector()
{
	if(!m->shortcut_connector){
		m->shortcut_connector = new DB::Shortcuts(this->connectionName(), this->databaseId());
	}

	return m->shortcut_connector;
}

DB::Library* Connector::libraryConnector()
{
	if(!m->library_connector){
		m->library_connector = new DB::Library(this->connectionName(), this->databaseId());
	}

	return m->library_connector;
}

DB::Covers* Connector::coverConnector()
{
	if(!m->cover_connector){
		m->cover_connector = new DB::Covers(this->connectionName(), this->databaseId());
	}

	return m->cover_connector;
}


DB::Session* DB::Connector::sessionConnector()
{
	if(!m->session_connector){
		m->session_connector = new DB::Session(this->connectionName(), this->databaseId());
	}

	return m->session_connector;
}


const char* DatabaseNotCreatedException::what() const noexcept
{
	return "Database could not be created";
}
