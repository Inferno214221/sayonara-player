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
#include "Database/Equalizer.h"
#include "Database/Playlist.h"
#include "Database/Podcasts.h"
#include "Database/Streams.h"
#include "Database/Session.h"
#include "Database/Settings.h"
#include "Database/Shortcuts.h"
#include "Database/VisualStyles.h"
#include "Database/CoverConnector.h"
#include "Database/SmartPlaylists.h"

#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/StandardPaths.h"
#include "Utils/Algorithm.h"
#include "Utils/RawShortcutMap.h"

#include <QFileInfo>
#include <QDateTime>
#include <QTime>

#include <tuple>
#include <algorithm>

using DB::Connector;
using DB::LibraryDatabase;

using LibDbIterator = DB::LibraryDatabases::Iterator;
namespace Algorithm = Util::Algorithm;

int Connector::highestDatabaseVersion()
{
	return 30;
}

struct Connector::Private
{
	QString connection_name;
	QString defaultSourcedirectory;
	QString defaultTargetdirectory;
	QString defaultDatabsefilename;

	DB::Bookmarks* bookmarkConnector = nullptr;
	DB::Equalizer* equalizerConnector = nullptr;
	DB::Playlist* playlistConnector = nullptr;
	DB::Podcasts* podcastConnector = nullptr;
	DB::Streams* streamConnector = nullptr;
	DB::VisualStyles* visualStyleConnector = nullptr;
	DB::Session* sessionConnector = nullptr;
	DB::Settings* settingsConnector = nullptr;
	DB::Shortcuts* shortcutConnector = nullptr;
	DB::Covers* coverConnector = nullptr;
	DB::Library* libraryConnector = nullptr;
	DB::SmartPlaylists* smartPlaylistConnector = nullptr;

	QList<LibraryDatabase*> libraryDbs;
	LibraryDatabase* genericLibraryDatabase = nullptr;

	int oldDbVersion;

	Private() :
		oldDbVersion(0) {}

	~Private()
	{
		if(bookmarkConnector)
		{
			delete bookmarkConnector;
			bookmarkConnector = nullptr;
		}

		if(equalizerConnector)
		{
			delete equalizerConnector;
			equalizerConnector = nullptr;
		}

		if(podcastConnector)
		{
			delete podcastConnector;
			podcastConnector = nullptr;
		}

		if(streamConnector)
		{
			delete streamConnector;
			streamConnector = nullptr;
		}

		if(visualStyleConnector)
		{
			delete visualStyleConnector;
			visualStyleConnector = nullptr;
		}

		if(settingsConnector)
		{
			delete settingsConnector;
			settingsConnector = nullptr;
		}

		if(shortcutConnector)
		{
			delete shortcutConnector;
			shortcutConnector = nullptr;
		}

		if(coverConnector)
		{
			delete coverConnector;
			coverConnector = nullptr;
		}

		if(libraryConnector)
		{
			delete libraryConnector;
			libraryConnector = nullptr;
		}

		if(sessionConnector)
		{
			delete sessionConnector;
			sessionConnector = nullptr;
		}

		if(smartPlaylistConnector)
		{
			delete smartPlaylistConnector;
			smartPlaylistConnector = nullptr;
		}
	}
};

class DatabaseNotCreatedException :
	public std::exception
{
	public:
		const char* what() const noexcept;
};

Connector::Connector(const QString& sourceDirectory, const QString& targetDirectory, const QString& databseFilename) :
	DB::Base(0, sourceDirectory, targetDirectory, databseFilename, nullptr)
{
	m = Pimpl::make<Private>();

	if(!this->isInitialized())
	{
		throw DatabaseNotCreatedException();
	}

	else
	{
		m->genericLibraryDatabase = new LibraryDatabase(connectionName(), databaseId(), -1);
		m->libraryDbs << m->genericLibraryDatabase;

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
	if(sourceDirectory.isEmpty())
	{
		sourceDirectory = ":/Database";
	}

	if(targetDirectory.isEmpty())
	{
		targetDirectory = Util::xdgConfigPath();
	}

	if(databseFilename.isEmpty())
	{
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

	for(const Album& album: albums)
	{
		QString str = "UPDATE albums SET cissearch=:cissearch WHERE albumID=:id;";
		Query q(this);
		q.prepare(str);
		q.bindValue(":cissearch", Util::convertNotNull(album.name().toLower()));
		q.bindValue(":id", album.id());

		if(!q.exec())
		{
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
	for(const Artist& artist: artists)
	{
		QString str =
			"UPDATE artists SET cissearch=:cissearch WHERE artistID=:id;";

		Query q(this);
		q.prepare(str);
		q.bindValue(":cissearch", Util::convertNotNull(artist.name().toLower()));
		q.bindValue(":id", artist.id());

		if(!q.exec())
		{
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
	for(const MetaData& md: v_md)
	{
		lib_db->updateTrack(md);
	}

	return true;
}

bool Connector::updateLostArtists()
{
	LibraryDatabase* lib_db = libraryDatabase(-1, 0);
	if(!lib_db)
	{
		spLog(Log::Error, this) << "Cannot find Library";
		return false;
	}

	ArtistId id = lib_db->insertArtistIntoDatabase(QString());

	const QStringList queries {
		QString(
			"UPDATE tracks SET artistID=:artistID WHERE artistID IN (SELECT artistID FROM artists WHERE name IS NULL);"),
		QString("UPDATE tracks SET artistID=:artistID WHERE artistID NOT IN (SELECT artistID FROM artists);"),
		QString(
			"UPDATE tracks SET albumArtistID=:artistID WHERE albumArtistID IN (SELECT artistID FROM artists WHERE name IS NULL);"),
		QString("UPDATE tracks SET albumArtistID=:artistID WHERE albumArtistID NOT IN (SELECT artistID FROM artists);"),
		QString("DELETE FROM artists WHERE name IS NULL;")
	};

	this->transaction();
	for(const QString& query: queries)
	{
		DB::Query q(this);
		q.prepare(query);
		q.bindValue(":artistID", id);
		bool success = q.exec();
		if(!success)
		{
			this->rollback();
			return false;
		}
	}

	this->commit();
	return true;
}

bool Connector::updateLostAlbums()
{
	LibraryDatabase* libraryDatabase = this->libraryDatabase(-1, 0);
	if(!libraryDatabase)
	{
		spLog(Log::Error, this) << "Cannot find Library database";
		return false;
	}

	return libraryDatabase->fixEmptyAlbums();
}

int Connector::oldDatabaseVersion() const
{
	return m->oldDbVersion;
}

bool Connector::applyFixes()
{
	QString versionString;

	const int LatestVersion = highestDatabaseVersion();

	auto success = settingsConnector()->loadSetting("version", versionString);
	const auto version = versionString.toInt(&success);
	
	m->oldDbVersion = version;

	spLog(Log::Info, this)
		<< "Database Version:  " << version << ". "
		<< "Latest Version: " << LatestVersion;

	if(version == LatestVersion)
	{
		spLog(Log::Info, this) << "No need to update db";
		return true;
	}

	else if(!success)
	{
		spLog(Log::Warning, this) << "Cannot get database version";
	}

	settingsConnector()->loadSettings();

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

	if(version == 3)
	{
		checkAndDropTable("VisualStyles");
	}

	if(version < 4)
	{
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
		if(success) { settingsConnector()->storeSetting("version", 4); }
	}

	if(version < 5)
	{
		bool success = checkAndInsertColumn("tracks", "rating", "integer");
		if(success) { settingsConnector()->storeSetting("version", 5); }
	}

	if(version < 6)
	{
		QString create_savedbookmarks = QString("CREATE TABLE savedbookmarks ") +
		                                "( " +
		                                "	trackid INTEGER, " +
		                                "	name VARCHAR(255), " +
		                                "	timeidx INTEGER, " +
		                                "   PRIMARY KEY (trackid, timeidx), " +
		                                "   FOREIGN KEY (trackid) REFERENCES tracks(trackid) " +
		                                ");";

		bool success = checkAndCreateTable("savedbookmarks", create_savedbookmarks);
		if(success) { settingsConnector()->storeSetting("version", 6); }
	}

	if(version < 7)
	{
		bool success = checkAndInsertColumn("albums", "rating", "integer");
		if(success) { settingsConnector()->storeSetting("version", 7); }
	}

	if(version < 9)
	{
		bool success = checkAndInsertColumn("playlists", "temporary", "integer");

		if(success)
		{
			Query q(this);
			QString querytext = "UPDATE playlists SET temporary=0;";
			q.prepare(querytext);
			if(q.exec())
			{
				settingsConnector()->storeSetting("version", 9);
			};
		}
	}

	if(version < 10)
	{
		bool success = checkAndInsertColumn("playlisttotracks", "db_id", "integer");
		if(success)
		{
			Query q(this);
			Query q_index(this);
			QString querytext = "UPDATE playlisttotracks SET db_id = (CASE WHEN trackid > 0 THEN 0 ELSE -1 END)";
			QString index_query = "CREATE INDEX album_search ON albums(cissearch, albumID);"
			                      "CREATE INDEX artist_search ON artists(cissearch, artistID);"
			                      "CREATE INDEX track_search ON tracks(cissearch, trackID);";

			q.prepare(querytext);
			q_index.prepare(index_query);

			if(q.exec())
			{
				settingsConnector()->storeSetting("version", 10);
			};

			q_index.exec();
		}
	}

	if(version < 11)
	{
		// look in UpdateDatesThread
	}

	if(version < 12)
	{
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
			"GROUP BY albums.albumID, albums.name";;

		Query q(this);
		q.prepare(querytext);

		if(q.exec())
		{
			settingsConnector()->storeSetting("version", 12);
		}
	}

	if(version < 13)
	{
		bool success = checkAndInsertColumn("tracks", "albumArtistID", "integer", "-1");

		Query q(this);
		q.prepare("UPDATE tracks SET albumArtistID=artistID;");
		success = success && q.exec();

		if(success)
		{
			settingsConnector()->storeSetting("version", 13);
		}
	}

	if(version < 14)
	{
		bool success = checkAndInsertColumn("tracks", "libraryID", "integer", "0");
		Query q(this);
		q.prepare("UPDATE tracks SET libraryID=0;");
		success = success && q.exec();

		if(success)
		{
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

		bool success = checkAndCreateTable("Libraries", create_string);
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
			for(const MetaData& md: v_md)
			{
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
			for(const QString& key: rsm.keys())
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

		for(auto it = tracks.begin(); it != tracks.end(); it++)
		{
			albums[it->album()] = it->albumId();
			artists[it->artist()] = it->artistId();
			artists[it->albumArtist()] = it->albumArtistId();
		}

		for(auto it = tracks.begin(); it != tracks.end(); it++)
		{
			AlbumId correct_albumId = albums[it->album()];
			ArtistId correct_artistId = artists[it->artist()];
			ArtistId correct_album_artistId = artists[it->albumArtist()];
			this->transaction();
			if((it->albumId() != correct_albumId) ||
			   (it->artistId() != correct_artistId) ||
			   (it->albumArtistId() != correct_album_artistId))
			{
				spLog(Log::Info, this) << "Move track " << it->filepath() << "from album " << it->albumId() << " to "
				                       << correct_albumId;

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
			QString query(
				"DELETE FROM artists WHERE artists.artistID NOT IN (SELECT artistId from tracks UNION SELECT albumArtistId FROM tracks);");
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

	if(version < 26)
	{
		checkAndInsertColumn("playlistToTracks", "stationName", "VARCHAR(255)");
		checkAndInsertColumn("playlistToTracks", "station", "VARCHAR(512)");
		checkAndInsertColumn("playlistToTracks", "isRadio", "INTEGER", "0");

		settingsConnector()->storeSetting("version", 26);
	}

	if(version < 27)
	{
		bool success = checkAndInsertColumn("tracks", "genreCissearch", "VARCHAR(512)");

		if(success)
		{
			settingsConnector()->storeSetting("version", 27);

			MetaDataList tracks;
			LibraryDatabase* libraryDb = new DB::LibraryDatabase(connectionName(), databaseId(), -1);
			libraryDb->getAllTracks(tracks);
			this->transaction();
			for(const MetaData& md: tracks)
			{
				libraryDb->updateTrack(md);
			}
			this->commit();

			delete libraryDb;
		}
	}

	if(version < 28)
	{
		bool success = checkAndInsertColumn("playlistToTracks", "coverDownloadUrl", "VARCHAR(512)");
		if(success)
		{
			settingsConnector()->storeSetting("version", 28);
		}
	}

	if(version < 29)
	{
		const auto createStatement = R"create(
			CREATE TABLE Equalizer
            (
              id INTEGER PRIMARY KEY AUTOINCREMENT,
              name VARCHAR(32),
              equalizerValues VARCHAR(32),
              defaultValues VARCHAR(32)
            );
			)create";

		bool success = checkAndCreateTable("Equalizer", createStatement);
		success &= equalizerConnector()->restoreFactoryDefaults();

		if(success)
		{
			settingsConnector()->storeSetting("version", 29);
		}
	}

	if(version < 30)
	{
		const auto createStatement = R"create(
			CREATE TABLE SmartPlaylists
            (
              id INTEGER PRIMARY KEY AUTOINCREMENT,
              attributes VARCHAR(128),
              classType VARCHAR(32)
			);
			)create";

		const auto success = checkAndCreateTable("SmartPlaylists", createStatement);
		if(success)
		{
			settingsConnector()->storeSetting("version", 30);
		}

	}

	return true;
}

DB::LibraryDatabases Connector::libraryDatabases() const
{
	return m->libraryDbs;
}

DB::LibraryDatabase* Connector::libraryDatabase(LibraryId libraryId, DbId databaseId)
{
	LibDbIterator it = Algorithm::find(m->libraryDbs, [&](DB::LibraryDatabase* db) {
		return (db->libraryId() == libraryId && db->databaseId() == databaseId);
	});

	if(it == m->libraryDbs.end())
	{
		spLog(Log::Warning, this) << "Could not find Library:"
		                             " DB ID = " << int(databaseId)
		                          << " LibraryID = " << int(libraryId);

		return m->genericLibraryDatabase;
	}

	return *it;
}

DB::LibraryDatabase* Connector::registerLibraryDatabase(LibraryId libraryId)
{
	DB::LibraryDatabase* lib_db = nullptr;
	LibDbIterator it = Algorithm::find(m->libraryDbs, [=](DB::LibraryDatabase* db) {
		return (db->libraryId() == libraryId);
	});

	if(it == m->libraryDbs.end())
	{
		lib_db = new DB::LibraryDatabase(this->connectionName(), this->databaseId(), libraryId);
		m->libraryDbs << lib_db;
	}

	else
	{
		lib_db = *it;
	}

	return lib_db;
}

void Connector::deleteLibraryDatabase(LibraryId libraryId)
{
	LibDbIterator it = Algorithm::find(m->libraryDbs, [=](DB::LibraryDatabase* db) {
		return (db->libraryId() == libraryId);
	});

	if(it != m->libraryDbs.end())
	{
		LibraryDatabase* db = *it;
		db->deleteAllTracks(true);
		m->libraryDbs.removeAll(db);

		delete db;
		db = nullptr;
	}
}

DB::Playlist* Connector::playlistConnector()
{
	if(!m->playlistConnector)
	{
		m->playlistConnector = new DB::Playlist(this->connectionName(), this->databaseId());
	}

	return m->playlistConnector;
}

DB::Bookmarks* Connector::bookmarkConnector()
{
	if(!m->bookmarkConnector)
	{
		m->bookmarkConnector = new DB::Bookmarks(this->connectionName(), this->databaseId());
	}

	return m->bookmarkConnector;
}

DB::Streams* Connector::streamConnector()
{
	if(!m->streamConnector)
	{
		m->streamConnector = new DB::Streams(this->connectionName(), this->databaseId());
	}

	return m->streamConnector;
}

DB::Podcasts* Connector::podcastConnector()
{
	if(!m->podcastConnector)
	{
		m->podcastConnector = new DB::Podcasts(this->connectionName(), this->databaseId());
	}

	return m->podcastConnector;
}

DB::VisualStyles* Connector::visualStyleConnector()
{
	if(!m->visualStyleConnector)
	{
		m->visualStyleConnector = new DB::VisualStyles(this->connectionName(), this->databaseId());
	}

	return m->visualStyleConnector;
}

DB::Settings* Connector::settingsConnector()
{
	if(!m->settingsConnector)
	{
		m->settingsConnector = new DB::Settings(this->connectionName(), this->databaseId());
	}

	return m->settingsConnector;
}

DB::Shortcuts* Connector::shortcutConnector()
{
	if(!m->shortcutConnector)
	{
		m->shortcutConnector = new DB::Shortcuts(this->connectionName(), this->databaseId());
	}

	return m->shortcutConnector;
}

DB::Library* Connector::libraryConnector()
{
	if(!m->libraryConnector)
	{
		m->libraryConnector = new DB::Library(this->connectionName(), this->databaseId());
	}

	return m->libraryConnector;
}

DB::Covers* Connector::coverConnector()
{
	if(!m->coverConnector)
	{
		m->coverConnector = new DB::Covers(this->connectionName(), this->databaseId());
	}

	return m->coverConnector;
}

DB::Session* DB::Connector::sessionConnector()
{
	if(!m->sessionConnector)
	{
		m->sessionConnector = new DB::Session(this->connectionName(), this->databaseId());
	}

	return m->sessionConnector;
}

DB::Equalizer* DB::Connector::equalizerConnector()
{
	if(!m->equalizerConnector)
	{
		m->equalizerConnector = new DB::Equalizer(this->connectionName(), this->databaseId());
	}

	return m->equalizerConnector;
}

DB::SmartPlaylists* DB::Connector::smartPlaylistsConnector()
{
	if(!m->smartPlaylistConnector)
	{
		m->smartPlaylistConnector = new DB::SmartPlaylists(connectionName(), databaseId());
	}

	return m->smartPlaylistConnector;
}

const char* DatabaseNotCreatedException::what() const noexcept
{
	return "Database could not be created";
}
