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

namespace Algorithm = Util::Algorithm;

int Connector::highestDatabaseVersion()
{
	return 32;  // NOLINT(readability-magic-numbers)
}

struct Connector::Private
{
	std::unique_ptr<DB::Bookmarks> bookmarkConnector {nullptr};
	std::unique_ptr<DB::Equalizer> equalizerConnector {nullptr};
	std::unique_ptr<DB::Playlist> playlistConnector {nullptr};
	std::unique_ptr<DB::Podcasts> podcastConnector {nullptr};
	std::unique_ptr<DB::Streams> streamConnector {nullptr};
	std::unique_ptr<DB::VisualStyles> visualStyleConnector {nullptr};
	std::unique_ptr<DB::Session> sessionConnector {nullptr};
	std::unique_ptr<DB::Settings> settingsConnector {nullptr};
	std::unique_ptr<DB::Shortcuts> shortcutConnector {nullptr};
	std::unique_ptr<DB::Covers> coverConnector {nullptr};
	std::unique_ptr<DB::Library> libraryConnector {nullptr};
	std::unique_ptr<DB::SmartPlaylists> smartPlaylistConnector {nullptr};

	QList<LibraryDatabase*> libraryDbs;
	LibraryDatabase* genericLibraryDatabase {nullptr};

	int oldDbVersion {0};
};

class DatabaseNotCreatedException :
	public std::exception
{
	public:
		[[nodiscard]] const char* what() const noexcept override;
};

Connector::Connector(const QString& sourceDirectory, const QString& targetDirectory, const QString& databseFilename) :
	DB::Base(0, sourceDirectory, targetDirectory, databseFilename, nullptr)
{
	m = Pimpl::make<Private>();

	if(!this->isInitialized())
	{
		throw DatabaseNotCreatedException();
	}

	m->genericLibraryDatabase = new LibraryDatabase(connectionName(), databaseId(), -1);
	m->libraryDbs << m->genericLibraryDatabase;

	applyFixes();
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
	AlbumList albums;

	auto* libraryDatabase = this->libraryDatabase(-1, 0);
	libraryDatabase->getAllAlbums(albums, true);

	for(const auto& album: albums)
	{
		update("albums",
		       {{"cissearch", Util::convertNotNull(album.name().toLower())}},
		       {"albumID", album.id()},
		       "Cannot update album cissearch");
	}

	return true;
}

bool Connector::updateArtistCissearchFix()
{
	ArtistList artists;
	auto* libraryDatabase = this->libraryDatabase(-1, 0);
	libraryDatabase->getAllArtists(artists, true);

	for(const auto& artist: artists)
	{
		update("artists",
		       {{"cissearch", Util::convertNotNull(artist.name().toLower())}},
		       {"artistID", artist.id()},
		       "Cannot update artist cissearch");
	}

	return true;
}

bool Connector::updateTrackCissearchFix()
{
	auto tracks = MetaDataList {};
	auto* libraryDatabase = this->libraryDatabase(-1, 0);
	libraryDatabase->getAllTracks(tracks);

	for(const auto& track: tracks)
	{
		libraryDatabase->updateTrack(track);
	}

	return true;
}

bool Connector::updateLostArtists()
{
	auto* libraryDatabase = this->libraryDatabase(-1, 0);
	if(!libraryDatabase)
	{
		spLog(Log::Error, this) << "Cannot find Library";
		return false;
	}

	const auto id = libraryDatabase->insertArtistIntoDatabase(QString());

	const auto queries = QStringList {
		"UPDATE tracks SET artistID=:artistID WHERE artistID IN (SELECT artistID FROM artists WHERE name IS NULL);",
		"UPDATE tracks SET artistID=:artistID WHERE artistID NOT IN (SELECT artistID FROM artists);",
		"UPDATE tracks SET albumArtistID=:artistID WHERE albumArtistID IN (SELECT artistID FROM artists WHERE name IS NULL);",
		"UPDATE tracks SET albumArtistID=:artistID WHERE albumArtistID NOT IN (SELECT artistID FROM artists);",
		"DELETE FROM artists WHERE name IS NULL;"
	};

	transaction();
	for(const auto& query: queries)
	{
		auto q = runQuery(query, {":artistID", id}, "Cannot update lost artist");
		if(hasError(q))
		{
			rollback();
			return false;
		}
	}

	commit();
	return true;
}

bool Connector::updateLostAlbums()
{
	auto* libraryDatabase = this->libraryDatabase(-1, 0);
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

bool Connector::applyFixes() // NOLINT(readability-function-cognitive-complexity)
{
	QString versionString;

	const auto LatestVersion = highestDatabaseVersion();

	auto loadingSuccessful = settingsConnector()->loadSetting("version", versionString);
	const auto version = versionString.toInt(&loadingSuccessful);

	m->oldDbVersion = version;

	spLog(Log::Info, this)
		<< "Database Version:  " << version << ". "
		<< "Latest Version: " << LatestVersion;

	if(version == LatestVersion)
	{
		spLog(Log::Info, this) << "No need to update db";
		return true;
	}

	if(!loadingSuccessful)
	{
		spLog(Log::Warning, this) << "Cannot get database version";
	}

	settingsConnector()->loadSettings();

	spLog(Log::Info, this) << "Apply fixes";

	if(version < 1) // NOLINT(readability-magic-numbers)
	{
		checkAndInsertColumn("playlisttotracks", "position", "INTEGER");
		checkAndInsertColumn("playlisttotracks", "filepath", "VARCHAR(512)");
		checkAndInsertColumn("tracks", "genre", "VARCHAR(1024)");

		checkAndCreateTable("savedstreams", R"(CREATE TABLE savedstreams
			(
				name VARCHAR(255) PRIMARY KEY,
				url VARCHAR(255)
			);)"
		);

		checkAndCreateTable("savedpodcasts", R"(CREATE TABLE savedpodcasts
			(
				name VARCHAR(255) PRIMARY KEY,
				url VARCHAR(255)
			);)"
		);
	}

	if(version < 3) // NOLINT(readability-magic-numbers)
	{
		db().transaction();

		checkAndInsertColumn("tracks", "cissearch", "VARCHAR(512)");
		checkAndInsertColumn("albums", "cissearch", "VARCHAR(512)");
		checkAndInsertColumn("artists", "cissearch", "VARCHAR(512)");

		updateAlbumCissearchFix();
		updateArtistCissearchFix();
		updateTrackCissearchFix();

		db().commit();
	}

	if(version == 3) // NOLINT(readability-magic-numbers)
	{
		checkAndDropTable("VisualStyles");
	}

	if(version < 4) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndCreateTable("VisualStyles", R"(CREATE TABLE VisualStyles
			(
				name VARCHAR(255) PRIMARY KEY,
				col1 VARCHAR(20),
				col2 VARCHAR(20),
				col3 VARCHAR(20),
				col4 VARCHAR(20),
				nBinsSpectrum INTEGER,
				rectHeightSpectrum INTEGER,
				fadingStepsSpectrum INTEGER,
				horSpacingSpectrum INTEGER,
				vertSpacingSpectrum INTEGER,
				rectWidthLevel INTEGER,
				rectHeightLevel INTEGER,
				horSpacingLevel INTEGER,
				verSpacingLevel INTEGER,
				fadingStepsLevel INTEGER
			);)"
		);

		if(success)
		{
			settingsConnector()->storeSetting("version", 4);  // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 5) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndInsertColumn("tracks", "rating", "integer");
		if(success)
		{
			settingsConnector()->storeSetting("version", 5); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 6) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndCreateTable("savedbookmarks", R"(CREATE TABLE savedbookmarks
			(
				trackid INTEGER,
				name VARCHAR(255),
				timeidx INTEGER,
				PRIMARY KEY (trackid, timeidx),
				FOREIGN KEY (trackid) REFERENCES tracks(trackid)
			);)"
		);

		if(success)
		{
			settingsConnector()->storeSetting("version", 6); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 7) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndInsertColumn("albums", "rating", "integer");
		if(success)
		{
			settingsConnector()->storeSetting("version", 7); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 9) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndInsertColumn("playlists", "temporary", "integer");
		if(success)
		{
			auto q = QSqlQuery(db());
			const auto* querytext = "UPDATE playlists SET temporary=0;";
			q.prepare(querytext);
			if(q.exec())
			{
				settingsConnector()->storeSetting("version", 9); // NOLINT(readability-magic-numbers)
			};
		}
	}

	if(version < 10) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndInsertColumn("playlisttotracks", "db_id", "integer");
		if(success)
		{
			const auto* querytext = "UPDATE playlisttotracks SET db_id = (CASE WHEN trackid > 0 THEN 0 ELSE -1 END);";

			auto query = QSqlQuery(db());
			query.prepare(querytext);
			if(query.exec())
			{
				settingsConnector()->storeSetting("version", 10); // NOLINT(readability-magic-numbers)
			};

			const auto indexQueries = QStringList {
				"CREATE INDEX album_search ON albums(cissearch, albumID);",
				"CREATE INDEX artist_search ON artists(cissearch, artistID);",
				"CREATE INDEX track_search ON tracks(cissearch, trackID);"
			};

			for(const auto& queryTextIndex: indexQueries)
			{
				runQuery(queryTextIndex, "Cannot create index");
			}
		}
	}

	if(version < 11) // NOLINT(readability-magic-numbers)
	{
		// look in UpdateDatesThread
	}

	if(version < 12) // NOLINT(readability-magic-numbers)
	{
		const auto* querytext =
			R"(CREATE VIEW album_info_view AS
				SELECT
					albums.albumID as albumID,
					albums.name as name,
					albums.cissearch as cissearch,
					albums.rating as rating,
					COUNT(artists.artistID) as artistCount,
					COUNT(tracks.trackID) as trackCount,
					CASE WHEN COUNT(DISTINCT artists.artistID) > 1
					THEN 1
					ELSE 0
					END as Sampler
				FROM albums, artists, tracks
				WHERE albums.albumID = tracks.albumID
				AND artists.artistID = tracks.artistID
				GROUP BY albums.albumID, albums.name;)";

		auto q = QSqlQuery(db());
		q.prepare(querytext);

		if(q.exec())
		{
			settingsConnector()->storeSetting("version", 12); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 13) // NOLINT(readability-magic-numbers)
	{
		auto success = checkAndInsertColumn("tracks", "albumArtistID", "integer", "-1");

		auto q = QSqlQuery(db());
		q.prepare("UPDATE tracks SET albumArtistID=artistID;");
		success = success && q.exec();

		if(success)
		{
			settingsConnector()->storeSetting("version", 13); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 14) // NOLINT(readability-magic-numbers)
	{
		auto success = checkAndInsertColumn("tracks", "libraryID", "integer", "0");

		auto q = QSqlQuery(db());
		q.prepare("UPDATE tracks SET libraryID=0;");
		success = success && q.exec();

		if(success)
		{
			settingsConnector()->storeSetting("version", 14); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 15) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndCreateTable("Libraries", R"(CREATE TABLE Libraries
			(
				libraryID INTEGER NOT NULL,
				libraryName VARCHAR(128) NOT NULL,
				libraryPath VARCHAR(512) NOT NULL,
				libraryIndex INTEGER NOT NULL,
				PRIMARY KEY (libraryID, libraryPath)
			);)"
		);

		if(success)
		{
			settingsConnector()->storeSetting("version", 15); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 16) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndInsertColumn("tracks", "fileCissearch", "VARCHAR(256)");
		if(success)
		{
			settingsConnector()->storeSetting("version", 16); // NOLINT(readability-magic-numbers)

			auto tracks = MetaDataList {};
			auto* libraryDatabase = new DB::LibraryDatabase(connectionName(), databaseId(), -1);
			libraryDatabase->getAllTracks(tracks);

			this->transaction();
			for(const auto& track: tracks)
			{
				libraryDatabase->updateTrack(track);
			}
			this->commit();

			delete libraryDatabase;
		}
	}

	if(version < 17) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndInsertColumn("tracks", "comment", "VARCHAR(1024)");
		if(success)
		{
			settingsConnector()->storeSetting("version", 17); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 18) // NOLINT(readability-magic-numbers)
	{
		if(updateLostArtists() && updateLostAlbums())
		{
			settingsConnector()->storeSetting("version", 18); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 19) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndCreateTable("Shortcuts", R"(CREATE TABLE Shortcuts
			(
			  id INTEGER NOT NULL PRIMARY KEY,
			  identifier VARCHAR(32) NOT NULL,
			  shortcut VARCHAR(32) NOT NULL
			);)"
		);
		if(success)
		{
			auto raw = QString {};
			settingsConnector()->loadSetting("shortcuts", raw);

			const auto rsm = RawShortcutMap::fromString(raw);
			const auto keys = rsm.keys();
			for(const auto& key: keys)
			{
				this->shortcutConnector()->setShortcuts(key, rsm.value(key));
			}

			settingsConnector()->storeSetting("shortcuts", "<deprecated>");
			settingsConnector()->storeSetting("version", 19); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 20) // NOLINT(readability-magic-numbers)
	{
		checkAndDropTable("Covers");

		auto success = checkAndCreateTable("Covers", R"(CREATE TABLE Covers
			(
				coverId INTEGER PRIMARY KEY,
				hash VARCHAR(64),
				coverKey VARCHAR(128),
				data BLOB
			);)"
		);

		success &= checkAndCreateTable("TrackCoverMap", R"(CREATE TABLE TrackCoverMap
			(
				metadataId INTEGER,
				coverId INTEGER,
				PRIMARY KEY(metadataId, coverId),
				FOREIGN KEY(metadataId) REFERENCES Tracks(trackId) ON DELETE CASCADE,
				FOREIGN KEY(coverId) REFERENCES Covers(coverId) ON DELETE CASCADE
			);)"
		);

		success &= checkAndCreateTable("AlbumCoverMap", R"(CREATE TABLE AlbumCoverMap
			(
				albumId INTEGER,
				coverId INTEGER,
				PRIMARY KEY(albumId, coverId),
				FOREIGN KEY(albumId) REFERENCES Albums(albumId) ON DELETE CASCADE,
				FOREIGN KEY(coverId) REFERENCES Covers(coverId) ON DELETE CASCADE
			);)"
		);

		if(success)
		{
			settingsConnector()->storeSetting("version", 20); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 21) // NOLINT(readability-magic-numbers)
	{
		checkAndDropTable("Statistics");
		checkAndDropTable("Lyrics");
		checkAndDropTable("Genres");

		settingsConnector()->storeSetting("version", 21); // NOLINT(readability-magic-numbers)
	}

	if(version < 22) // NOLINT(readability-magic-numbers)
	{
		auto* libraryDatabase = this->libraryDatabase(-1, 0);

		auto albums = QMap<QString, AlbumId> {};
		auto artists = QMap<QString, ArtistId> {};
		auto tracks = MetaDataList {};

		libraryDatabase->getAllTracks(tracks);

		for(const auto& track: tracks)
		{
			albums[track.album()] = track.albumId();
			artists[track.artist()] = track.artistId();
			artists[track.albumArtist()] = track.albumArtistId();
		}

		for(auto track: tracks)
		{
			const auto correctAlbumId = albums[track.album()];
			const auto correctArtistId = artists[track.artist()];
			const auto correctAlbumArtistId = artists[track.albumArtist()];

			this->transaction();

			if((track.albumId() != correctAlbumId) ||
			   (track.artistId() != correctArtistId) ||
			   (track.albumArtistId() != correctAlbumArtistId))
			{
				spLog(Log::Info, this) << "Move track " << track.filepath() << "from album " << track.albumId()
				                       << " to "
				                       << correctAlbumId;

				track.setAlbumId(correctAlbumId);
				track.setArtistId(correctArtistId);
				track.setAlbumArtistId(correctAlbumArtistId);

				libraryDatabase->updateTrack(track);
			}

			this->commit();
		}

		const auto queries = QStringList {
			"DELETE FROM albums WHERE albums.albumID NOT IN (SELECT albumId from tracks);",
			"DELETE FROM artists WHERE artists.artistID NOT IN (SELECT artistId from tracks UNION SELECT albumArtistId FROM tracks);"
		};

		for(const auto& query: queries)
		{
			auto q = QSqlQuery(db());
			q.prepare(query);
			q.exec();
		}

		settingsConnector()->storeSetting("version", 22); // NOLINT(readability-magic-numbers)
	}

	if(version < 23) // NOLINT(readability-magic-numbers)
	{
		checkAndCreateTable("Sessions", R"(CREATE TABLE IF NOT EXISTS Sessions
			(
				sessionID INTEGER,
				date INTEGER,
				trackID INTEGER DEFAULT -1 REFERENCES Tracks(trackID) ON DELETE SET DEFAULT,
				title VARCHAR(128),
				artist VARCHAR(128),
				album VARCHAR(128)
			);)"
		);

		const auto* query =
			R"(INSERT INTO Sessions
			SELECT
				session.date,
				session.date,
				session.trackID,
				tracks.title,
				artists.name,
				albums.name
			FROM Session session, Tracks tracks, Artists artists, Albums albums
			WHERE tracks.artistID = artists.artistID
			AND tracks.albumID = albums.albumID
			AND Session.trackID = tracks.trackID;)";

		auto q = QSqlQuery(db());
		q.prepare(query);
		q.exec();

		checkAndDropTable("Session");
		settingsConnector()->storeSetting("version", 23); // NOLINT(readability-magic-numbers)
	}

	if(version < 24) // NOLINT(readability-magic-numbers)
	{
		const auto queries = QStringList {
			"UPDATE Sessions SET sessionID = (sessionID + 20000000000000) WHERE sessionID < 1000000000000;",
			"UPDATE Sessions SET date = (date + 20000000000000) WHERE date < 1000000000000;"
		};

		for(const auto& query: queries)
		{
			auto q = QSqlQuery(db());
			q.prepare(query);
			q.exec();
		}

		settingsConnector()->storeSetting("version", 24); // NOLINT(readability-magic-numbers)
	}

	if(version < 25) // NOLINT(readability-magic-numbers)
	{
		checkAndInsertColumn("savedpodcasts", "reversed", "INTEGER", "0");
		settingsConnector()->storeSetting("version", 25); // NOLINT(readability-magic-numbers)
	}

	if(version < 26) // NOLINT(readability-magic-numbers)
	{
		checkAndInsertColumn("playlistToTracks", "stationName", "VARCHAR(255)");
		checkAndInsertColumn("playlistToTracks", "station", "VARCHAR(512)");
		checkAndInsertColumn("playlistToTracks", "isRadio", "INTEGER", "0");

		settingsConnector()->storeSetting("version", 26); // NOLINT(readability-magic-numbers)
	}

	if(version < 27) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndInsertColumn("tracks", "genreCissearch", "VARCHAR(512)");
		if(success)
		{
			settingsConnector()->storeSetting("version", 27); // NOLINT(readability-magic-numbers)

			auto tracks = MetaDataList {};
			auto* libraryDb = new DB::LibraryDatabase(connectionName(), databaseId(), -1);
			libraryDb->getAllTracks(tracks);

			this->transaction();
			for(const auto& track: tracks)
			{
				libraryDb->updateTrack(track);
			}
			this->commit();

			delete libraryDb;
		}
	}

	if(version < 28) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndInsertColumn("playlistToTracks", "coverDownloadUrl", "VARCHAR(512)");
		if(success)
		{
			settingsConnector()->storeSetting("version", 28); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 29) // NOLINT(readability-magic-numbers)
	{
		auto success = checkAndCreateTable("Equalizer", R"(CREATE TABLE Equalizer
            (
              id INTEGER PRIMARY KEY AUTOINCREMENT,
              name VARCHAR(32),
              equalizerValues VARCHAR(32),
              defaultValues VARCHAR(32)
            );)"
		);

		success &= equalizerConnector()->restoreFactoryDefaults();

		if(success)
		{
			settingsConnector()->storeSetting("version", 29); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 30) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndCreateTable("SmartPlaylists", R"(CREATE TABLE SmartPlaylists
            (
              id INTEGER PRIMARY KEY AUTOINCREMENT,
              attributes VARCHAR(128),
              classType VARCHAR(32)
			);)"
		);

		if(success)
		{
			settingsConnector()->storeSetting("version", 30); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 31) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndInsertColumn("SmartPlaylists", "isRandomized", "integer", "1");
		if(success)
		{
			settingsConnector()->storeSetting("version", 31); // NOLINT(readability-magic-numbers)
		}
	}

	if(version < 32) // NOLINT(readability-magic-numbers)
	{
		const auto success = checkAndInsertColumn("SmartPlaylists", "libraryId", "integer", "-1");
		if(success)
		{
			settingsConnector()->storeSetting("version", 32); // NOLINT(readability-magic-numbers)
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
	const auto it = Algorithm::find(m->libraryDbs, [&](DB::LibraryDatabase* db) {
		return (db->libraryId() == libraryId && db->databaseId() == databaseId);
	});

	if(it == m->libraryDbs.end())
	{
		spLog(Log::Warning, this) << "Could not find Library:"
		                          << " DB ID = " << static_cast<int>(databaseId)
		                          << " LibraryID = " << static_cast<int>(libraryId);

		return m->genericLibraryDatabase;
	}

	return *it;
}

DB::LibraryDatabase* Connector::registerLibraryDatabase(LibraryId libraryId)
{
	const auto it = Algorithm::find(m->libraryDbs, [=](DB::LibraryDatabase* db) {
		return (db->libraryId() == libraryId);
	});

	if(it == m->libraryDbs.end())
	{
		auto* libraryDatabase = new DB::LibraryDatabase(this->connectionName(), this->databaseId(), libraryId);
		m->libraryDbs << libraryDatabase;
		return libraryDatabase;
	}

	return *it;
}

void Connector::deleteLibraryDatabase(LibraryId libraryId)
{
	const auto it = Algorithm::find(m->libraryDbs, [=](DB::LibraryDatabase* db) {
		return (db->libraryId() == libraryId);
	});

	if(it != m->libraryDbs.end())
	{
		auto* libraryDatabase = *it;
		libraryDatabase->deleteAllTracks(true);
		m->libraryDbs.removeAll(libraryDatabase);

		delete libraryDatabase;
	}
}

DB::Playlist* Connector::playlistConnector()
{
	if(!m->playlistConnector)
	{
		m->playlistConnector = std::make_unique<DB::Playlist>(this->connectionName(), this->databaseId());
	}

	return m->playlistConnector.get();
}

DB::Bookmarks* Connector::bookmarkConnector()
{
	if(!m->bookmarkConnector)
	{
		m->bookmarkConnector = std::make_unique<DB::Bookmarks>(this->connectionName(), this->databaseId());
	}

	return m->bookmarkConnector.get();
}

DB::Streams* Connector::streamConnector()
{
	if(!m->streamConnector)
	{
		m->streamConnector = std::make_unique<DB::Streams>(this->connectionName(), this->databaseId());
	}

	return m->streamConnector.get();
}

DB::Podcasts* Connector::podcastConnector()
{
	if(!m->podcastConnector)
	{
		m->podcastConnector = std::make_unique<DB::Podcasts>(this->connectionName(), this->databaseId());
	}

	return m->podcastConnector.get();
}

DB::VisualStyles* Connector::visualStyleConnector()
{
	if(!m->visualStyleConnector)
	{
		m->visualStyleConnector = std::make_unique<DB::VisualStyles>(this->connectionName(), this->databaseId());
	}

	return m->visualStyleConnector.get();
}

DB::Settings* Connector::settingsConnector()
{
	if(!m->settingsConnector)
	{
		m->settingsConnector = std::make_unique<DB::Settings>(this->connectionName(), this->databaseId());
	}

	return m->settingsConnector.get();
}

DB::Shortcuts* Connector::shortcutConnector()
{
	if(!m->shortcutConnector)
	{
		m->shortcutConnector = std::make_unique<DB::Shortcuts>(this->connectionName(), this->databaseId());
	}

	return m->shortcutConnector.get();
}

DB::Library* Connector::libraryConnector()
{
	if(!m->libraryConnector)
	{
		m->libraryConnector = std::make_unique<DB::Library>(this->connectionName(), this->databaseId());
	}

	return m->libraryConnector.get();
}

DB::Covers* Connector::coverConnector()
{
	if(!m->coverConnector)
	{
		m->coverConnector = std::make_unique<DB::Covers>(this->connectionName(), this->databaseId());
	}

	return m->coverConnector.get();
}

DB::Session* DB::Connector::sessionConnector()
{
	if(!m->sessionConnector)
	{
		m->sessionConnector = std::make_unique<DB::Session>(this->connectionName(), this->databaseId());
	}

	return m->sessionConnector.get();
}

DB::Equalizer* DB::Connector::equalizerConnector()
{
	if(!m->equalizerConnector)
	{
		m->equalizerConnector = std::make_unique<DB::Equalizer>(this->connectionName(), this->databaseId());
	}

	return m->equalizerConnector.get();
}

DB::SmartPlaylists* DB::Connector::smartPlaylistsConnector()
{
	if(!m->smartPlaylistConnector)
	{
		m->smartPlaylistConnector = std::make_unique<DB::SmartPlaylists>(connectionName(), databaseId());
	}

	return m->smartPlaylistConnector.get();
}

const char* DatabaseNotCreatedException::what() const noexcept
{
	return "Database could not be created";
}
