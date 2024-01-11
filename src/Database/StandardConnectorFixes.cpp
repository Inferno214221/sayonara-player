/* StandardConnectorFixes.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "StandardConnectorFixes.h"
#include "Database/Equalizer.h"
#include "Database/LibraryDatabase.h"
#include "Database/Module.h"
#include "Database/Query.h"
#include "Database/Settings.h"
#include "Database/Shortcuts.h"

#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/RawShortcutMap.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

namespace
{
	constexpr const auto LatestDatabaseVersion = 35;

	bool updateAlbumCissearchFix(DB::Module& module, DB::LibraryDatabase& libraryDatabase)
	{
		AlbumList albums;
		libraryDatabase.getAllAlbums(albums, true);

		for(const auto& album: albums)
		{
			module.update("albums",
			              {{"cissearch", Util::convertNotNull(album.name().toLower())}},
			              {"albumID", album.id()},
			              "Cannot update album cissearch");
		}

		return true;
	}

	bool updateArtistCissearchFix(DB::Module& module, DB::LibraryDatabase& libraryDatabase)
	{
		ArtistList artists;
		libraryDatabase.getAllArtists(artists, true);

		for(const auto& artist: artists)
		{
			module.update("artists",
			              {{"cissearch", Util::convertNotNull(artist.name().toLower())}},
			              {"artistID", artist.id()},
			              "Cannot update artist cissearch");
		}

		return true;
	}

	bool updateTrackCissearchFix(DB::LibraryDatabase& libraryDatabase)
	{
		auto tracks = MetaDataList {};
		libraryDatabase.getAllTracks(tracks);

		for(const auto& track: tracks)
		{
			libraryDatabase.updateTrack(track);
		}

		return true;
	}

	bool updateLostArtists(QSqlDatabase db, DB::Module& module, DB::LibraryDatabase& libraryDatabase)
	{
		const auto id = libraryDatabase.insertArtistIntoDatabase(QString());
		const auto queries = QStringList {
			"UPDATE tracks SET artistID=:artistID WHERE artistID IN (SELECT artistID FROM artists WHERE name IS NULL);",
			"UPDATE tracks SET artistID=:artistID WHERE artistID NOT IN (SELECT artistID FROM artists);",
			"UPDATE tracks SET albumArtistID=:artistID WHERE albumArtistID IN (SELECT artistID FROM artists WHERE name IS NULL);",
			"UPDATE tracks SET albumArtistID=:artistID WHERE albumArtistID NOT IN (SELECT artistID FROM artists);",
			"DELETE FROM artists WHERE name IS NULL;"
		};

		db.transaction();
		for(const auto& query: queries)
		{
			auto q = module.runQuery(query, {":artistID", id}, "Cannot update lost artist");
			if(DB::hasError(q))
			{
				db.rollback();
				return false;
			}
		}

		db.commit();
		return true;
	}

	bool updateLostAlbums(DB::LibraryDatabase& libraryDatabase)
	{
		return libraryDatabase.fixEmptyAlbums();
	}
}

namespace DB
{
	struct StandardConnectorFixes::Private
	{
		QString connectionName;
		DbId databaseId;

		Private(QString connectionName, DbId databaseId) :
			connectionName {std::move(connectionName)},
			databaseId(databaseId) {}
	};

	StandardConnectorFixes::StandardConnectorFixes(const QString& connectionName, const DbId databaseId) :
		Fixes {connectionName, databaseId},
		m {Pimpl::make<Private>(connectionName, databaseId)} {}

	StandardConnectorFixes::~StandardConnectorFixes() noexcept = default;

	void StandardConnectorFixes::applyFixes() // NOLINT(readability-function-cognitive-complexity)
	{
		auto settingsConnector = Settings {m->connectionName, m->databaseId};
		settingsConnector.loadSettings();

		if(!isUpdateNeeded())
		{
			return;
		}

		const auto currentVersion = currentDatabaseVersion();

		auto libraryDatabase = LibraryDatabase {m->connectionName, m->databaseId, -1};
		auto module = Module(m->connectionName, m->databaseId);
		auto db = module.db();

		spLog(Log::Info, this) << "Apply fixes";

		if(currentVersion < 1) // NOLINT(readability-magic-numbers)
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

		if(currentVersion < 3) // NOLINT(readability-magic-numbers)
		{
			db.transaction();

			checkAndInsertColumn("tracks", "cissearch", "VARCHAR(512)");
			checkAndInsertColumn("albums", "cissearch", "VARCHAR(512)");
			checkAndInsertColumn("artists", "cissearch", "VARCHAR(512)");

			updateAlbumCissearchFix(module, libraryDatabase);
			updateArtistCissearchFix(module, libraryDatabase);
			updateTrackCissearchFix(libraryDatabase);

			db.commit();
		}

		if(currentVersion == 3) // NOLINT(readability-magic-numbers)
		{
			checkAndDropTable("VisualStyles");
		}

		if(currentVersion < 4) // NOLINT(readability-magic-numbers)
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
				settingsConnector.storeSetting("version", 4);  // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 5) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("tracks", "rating", "integer");
			if(success)
			{
				settingsConnector.storeSetting("version", 5); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 6) // NOLINT(readability-magic-numbers)
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
				settingsConnector.storeSetting("version", 6); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 7) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("albums", "rating", "integer");
			if(success)
			{
				settingsConnector.storeSetting("version", 7); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 9) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("playlists", "temporary", "integer");
			if(success)
			{
				auto q = QSqlQuery(db);
				const auto* querytext = "UPDATE playlists SET temporary=0;";
				q.prepare(querytext);
				if(q.exec())
				{
					settingsConnector.storeSetting("version", 9); // NOLINT(readability-magic-numbers)
				};
			}
		}

		if(currentVersion < 10) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("playlisttotracks", "db_id", "integer");
			if(success)
			{
				const auto* querytext = "UPDATE playlisttotracks SET db_id = (CASE WHEN trackid > 0 THEN 0 ELSE -1 END);";

				auto query = QSqlQuery(db);
				query.prepare(querytext);
				if(query.exec())
				{
					settingsConnector.storeSetting("version", 10); // NOLINT(readability-magic-numbers)
				};

				const auto indexQueries = QStringList {
					"CREATE INDEX album_search ON albums(cissearch, albumID);",
					"CREATE INDEX artist_search ON artists(cissearch, artistID);",
					"CREATE INDEX track_search ON tracks(cissearch, trackID);"
				};

				for(const auto& queryTextIndex: indexQueries)
				{
					module.runQuery(queryTextIndex, "Cannot create index");
				}
			}
		}

		if(currentVersion < 11) // NOLINT(readability-magic-numbers)
		{
			// look in UpdateDatesThread
		}

		if(currentVersion < 12) // NOLINT(readability-magic-numbers)
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

			auto q = QSqlQuery(db);
			q.prepare(querytext);

			if(q.exec())
			{
				settingsConnector.storeSetting("version", 12); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 13) // NOLINT(readability-magic-numbers)
		{
			auto success = checkAndInsertColumn("tracks", "albumArtistID", "integer", "-1");

			auto q = QSqlQuery(db);
			q.prepare("UPDATE tracks SET albumArtistID=artistID;");
			success = success && q.exec();

			if(success)
			{
				settingsConnector.storeSetting("version", 13); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 14) // NOLINT(readability-magic-numbers)
		{
			auto success = checkAndInsertColumn("tracks", "libraryID", "integer", "0");

			auto q = QSqlQuery(db);
			q.prepare("UPDATE tracks SET libraryID=0;");
			success = success && q.exec();

			if(success)
			{
				settingsConnector.storeSetting("version", 14); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 15) // NOLINT(readability-magic-numbers)
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
				settingsConnector.storeSetting("version", 15); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 16) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("tracks", "fileCissearch", "VARCHAR(256)");
			if(success)
			{
				settingsConnector.storeSetting("version", 16); // NOLINT(readability-magic-numbers)

				auto tracks = MetaDataList {};
				libraryDatabase.getAllTracks(tracks);

				db.transaction();
				for(const auto& track: tracks)
				{
					libraryDatabase.updateTrack(track);
				}
				db.commit();
			}
		}

		if(currentVersion < 17) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("tracks", "comment", "VARCHAR(1024)");
			if(success)
			{
				settingsConnector.storeSetting("version", 17); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 18) // NOLINT(readability-magic-numbers)
		{
			if(updateLostArtists(db, module, libraryDatabase) && updateLostAlbums(libraryDatabase))
			{
				settingsConnector.storeSetting("version", 18); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 19) // NOLINT(readability-magic-numbers)
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
				auto shortcutConnector = DB::Shortcuts(m->connectionName, m->databaseId);
				auto raw = QString {};
				settingsConnector.loadSetting("shortcuts", raw);

				const auto rsm = RawShortcutMap::fromString(raw);
				const auto keys = rsm.keys();
				for(const auto& key: keys)
				{
					shortcutConnector.setShortcuts(key, rsm.value(key));
				}

				settingsConnector.storeSetting("shortcuts", "<deprecated>");
				settingsConnector.storeSetting("version", 19); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 20) // NOLINT(readability-magic-numbers)
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
				settingsConnector.storeSetting("version", 20); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 21) // NOLINT(readability-magic-numbers)
		{
			checkAndDropTable("Statistics");
			checkAndDropTable("Lyrics");
			checkAndDropTable("Genres");

			settingsConnector.storeSetting("version", 21); // NOLINT(readability-magic-numbers)
		}

		if(currentVersion < 22) // NOLINT(readability-magic-numbers)
		{
			auto albums = QMap<QString, AlbumId> {};
			auto artists = QMap<QString, ArtistId> {};
			auto tracks = MetaDataList {};

			libraryDatabase.getAllTracks(tracks);

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

				db.transaction();

				if((track.albumId() != correctAlbumId) ||
				   (track.artistId() != correctArtistId) ||
				   (track.albumArtistId() != correctAlbumArtistId))
				{
					spLog(Log::Info, this) << "Move track " << track.filepath() << "from album "
					                       << track.albumId()
					                       << " to "
					                       << correctAlbumId;

					track.setAlbumId(correctAlbumId);
					track.setArtistId(correctArtistId);
					track.setAlbumArtistId(correctAlbumArtistId);

					libraryDatabase.updateTrack(track);
				}

				db.commit();
			}

			const auto queries = QStringList {
				"DELETE FROM albums WHERE albums.albumID NOT IN (SELECT albumId from tracks);",
				"DELETE FROM artists WHERE artists.artistID NOT IN (SELECT artistId from tracks UNION SELECT albumArtistId FROM tracks);"
			};

			for(const auto& query: queries)
			{
				auto q = QSqlQuery(db);
				q.prepare(query);
				q.exec();
			}

			settingsConnector.storeSetting("version", 22); // NOLINT(readability-magic-numbers)
		}

		if(currentVersion < 23) // NOLINT(readability-magic-numbers)
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

			auto q = QSqlQuery(db);
			q.prepare(query);
			q.exec();

			checkAndDropTable("Session");
			settingsConnector.storeSetting("version", 23); // NOLINT(readability-magic-numbers)
		}

		if(currentVersion < 24) // NOLINT(readability-magic-numbers)
		{
			const auto queries = QStringList {
				"UPDATE Sessions SET sessionID = (sessionID + 20000000000000) WHERE sessionID < 1000000000000;",
				"UPDATE Sessions SET date = (date + 20000000000000) WHERE date < 1000000000000;"
			};

			for(const auto& query: queries)
			{
				auto q = QSqlQuery(db);
				q.prepare(query);
				q.exec();
			}

			settingsConnector.storeSetting("version", 24); // NOLINT(readability-magic-numbers)
		}

		if(currentVersion < 25) // NOLINT(readability-magic-numbers)
		{
			checkAndInsertColumn("savedpodcasts", "reversed", "INTEGER", "0");
			settingsConnector.storeSetting("version", 25); // NOLINT(readability-magic-numbers)
		}

		if(currentVersion < 26) // NOLINT(readability-magic-numbers)
		{
			checkAndInsertColumn("playlistToTracks", "stationName", "VARCHAR(255)");
			checkAndInsertColumn("playlistToTracks", "station", "VARCHAR(512)");
			checkAndInsertColumn("playlistToTracks", "isRadio", "INTEGER", "0");

			settingsConnector.storeSetting("version", 26); // NOLINT(readability-magic-numbers)
		}

		if(currentVersion < 27) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("tracks", "genreCissearch", "VARCHAR(512)");
			if(success)
			{
				settingsConnector.storeSetting("version", 27); // NOLINT(readability-magic-numbers)

				auto tracks = MetaDataList {};
				libraryDatabase.getAllTracks(tracks);

				db.transaction();
				for(const auto& track: tracks)
				{
					libraryDatabase.updateTrack(track);
				}
				db.commit();
			}
		}

		if(currentVersion < 28) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("playlistToTracks", "coverDownloadUrl", "VARCHAR(512)");
			if(success)
			{
				settingsConnector.storeSetting("version", 28); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 29) // NOLINT(readability-magic-numbers)
		{
			auto success = checkAndCreateTable("Equalizer", R"(CREATE TABLE Equalizer
            (
              id INTEGER PRIMARY KEY AUTOINCREMENT,
              name VARCHAR(32),
              equalizerValues VARCHAR(32),
              defaultValues VARCHAR(32)
            );)"
			);

			auto equalizerConnector = DB::Equalizer(m->connectionName, m->databaseId);
			success &= equalizerConnector.restoreFactoryDefaults();

			if(success)
			{
				settingsConnector.storeSetting("version", 29); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 30) // NOLINT(readability-magic-numbers)
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
				settingsConnector.storeSetting("version", 30); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 31) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("SmartPlaylists", "isRandomized", "integer", "1");
			if(success)
			{
				settingsConnector.storeSetting("version", 31); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 32) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("SmartPlaylists", "libraryId", "integer", "-1");
			if(success)
			{
				settingsConnector.storeSetting("version", 32); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 33) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("playlistToTracks", "isUpdatable", "INTEGER", "1");
			if(success)
			{
				settingsConnector.storeSetting("version", 33); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 34) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("savedstreams", "isUpdatable", "INTEGER", "1");
			if(success)
			{
				settingsConnector.storeSetting("version", 34); // NOLINT(readability-magic-numbers)
			}
		}

		if(currentVersion < 35) // NOLINT(readability-magic-numbers)
		{
			const auto success = checkAndInsertColumn("playlists", "isLocked", "INTEGER", "0");
			if(success)
			{
				settingsConnector.storeSetting("version", 35); // NOLINT(readability-magic-numbers)
			}
		}
	}

	[[nodiscard]] int StandardConnectorFixes::currentDatabaseVersion() const
	{
		QString version;
		auto settingsConnector = Settings {m->connectionName, m->databaseId};
		settingsConnector.loadSetting("version", version);

		return version.toInt();
	}

	[[nodiscard]] bool StandardConnectorFixes::isUpdateNeeded() const
	{
		QString versionString;
		auto currentVersion = currentDatabaseVersion();

		spLog(Log::Info, this)
			<< "Database Version:  " << currentVersion << ". "
			<< "Latest Version: " << LatestDatabaseVersion;

		if(currentVersion == LatestDatabaseVersion)
		{
			spLog(Log::Info, this) << "No need to update db";
			return false;
		}

		if(currentVersion < 0)
		{
			spLog(Log::Warning, this) << "Cannot get database version";
			return false;
		}

		return true;
	}

	int StandardConnectorFixes::latestDatabaseVersion()
	{
		return LatestDatabaseVersion;
	}
}