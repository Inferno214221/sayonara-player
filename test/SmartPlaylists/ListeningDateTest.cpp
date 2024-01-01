/*
 * Copyright (C) 2011-2023 Michael Lugmair
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

#include "Common/SayonaraTest.h"
#include "Common/DatabaseUtils.h"
#include "Common/FileSystemMock.h"

#include "Database/Connector.h"
#include "Database/Session.h"
#include "Database/LibraryDatabase.h"
#include "Interfaces/PlaylistInterface.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Algorithm.h"
#include "Components/SmartPlaylists/SmartPlaylistManager.h"
#include "Components/SmartPlaylists/SmartPlaylistByListeningDate.h"
#include "Components/SmartPlaylists/DateConverter.h"

#include <QMap>
#include <QStringList>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	class PlaylistCreatorMock :
		public PlaylistCreator
	{
		public:
			~PlaylistCreatorMock() override = default;

			PlaylistPtr playlist(int /*playlistIndex*/) override { return {}; }

			PlaylistPtr playlistById(int /*playlistId*/) override { return {}; }

			[[nodiscard]] QString requestNewPlaylistName(const QString& /*prefix*/) const override { return {}; }

			int createPlaylist(const MetaDataList& tracks, const QString& /*name*/, bool /*temporary*/) override
			{
				m_playlist = tracks;
				return 0;
			}

			int createPlaylist(const QStringList& /*pathList*/, const QString& /*name*/, bool /*temporary*/,
			                   Playlist::LocalPathPlaylistCreator* /*ignored*/) override { return 0; }

			int createPlaylist(const CustomPlaylist& /*customPlaylist*/) override { return 0; }

			int createEmptyPlaylist(bool /*override*/) override { return 0; }

			int createCommandLinePlaylist(const QStringList& /*pathList*/,
			                              Playlist::LocalPathPlaylistCreator* /*ignored*/) override { return 0; }

			[[nodiscard]] MetaDataList tracks() const { return m_playlist; }

		private:
			MetaDataList m_playlist;
	};

	constexpr const LibraryId testLibraryId = 0;

	struct MetaDataBlock
	{
		QString album;
		QString artist;
		QString title;
	};

	[[maybe_unused]] MetaData createTestTrack(const MetaDataBlock& data)
	{
		auto track = MetaData {};
		track.setTitle(data.title);
		track.setAlbum(data.album);
		track.setArtist(data.artist);

		const auto path = QString("/path/to/%1/%2/%3.mp3")
			.arg(data.artist)
			.arg(data.album)
			.arg(data.title);
		track.setFilepath(path);

		return track;
	}

	void cleanLibraryDatabase(DB::LibraryDatabase* db)
	{
		Test::DB::deleteAllAlbums(db);
		Test::DB::deleteAllArtists(db);
		Test::DB::deleteAllTracks(db);
	}

	void createLibraryDatabase()
	{
		auto* dbConnector = DB::Connector::instance();
		dbConnector->deleteLibraryDatabase(testLibraryId);
		dbConnector->registerLibraryDatabase(testLibraryId);

		auto* db = dbConnector->libraryDatabase(testLibraryId, 0);
		cleanLibraryDatabase(db);
	}

	void createTestLibrary(const QList<MetaDataBlock>& data, const DB::ArtistIdInfo::ArtistIdField artistIdField)
	{
		createLibraryDatabase();
		auto* db = DB::Connector::instance()->libraryDatabase(testLibraryId, 0);
		db->changeArtistIdField(artistIdField);

		auto tracks = MetaDataList {};
		Util::Algorithm::transform(data, tracks, [&](const auto& dataItem) {
			return createTestTrack(dataItem);
		});

		db->storeMetadata(tracks);

		QVERIFY(db->getNumTracks() == data.count());
	}

	void addTrackToSession(int sessionId, const MetaData& track, const QDateTime& dateTime)
	{
		auto sessionDb = DB::Session(DB::Connector::instance()->connectionName(),
		                             DB::Connector::instance()->databaseId());
		sessionDb.addTrack(sessionId, track, dateTime);
	}

	QDateTime yesterday()
	{
		return QDate::currentDate().addDays(-1).startOfDay();
	}

	QDateTime lastMonth()
	{
		return QDate::currentDate().addDays(-30).startOfDay(); // NOLINT(*-magic-numbers)
	}
}

class ListeningDateTest :
	public Test::Base
{
	Q_OBJECT

	public:
		ListeningDateTest() :
			Test::Base("ListeningDateTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void test()
		{
			createTestLibrary({
				                  {"album", "artist", "title1"},
				                  {"album", "artist", "title2"}
			                  }, DB::ArtistIdInfo::ArtistIdField::ArtistId);

			const auto fileSystem = std::make_shared<Test::FileSystemMock>(QMap<QString, QStringList> {
				{"/path/to/artist/album", {"title1.mp3", "title2.mp3"}}
			});

			auto* library = DB::Connector::instance()->libraryDatabase(testLibraryId, 0);
			auto tracks = MetaDataList {};
			library->getAllTracks(tracks);

			addTrackToSession(0, tracks[0], yesterday());
			addTrackToSession(0, MetaData {"/invalid/path.mp3"}, yesterday().addSecs(60)); // NOLINT(*-magic-numbers)
			addTrackToSession(1, tracks[1], lastMonth());

			struct TestCase
			{
				int from {0};
				int to {0};
				QStringList expectedPaths;
			};

			const auto testCases = std::array {
				TestCase {0, 0, {}},
				TestCase {7, 10, {}},
				TestCase {0, 1, {"/path/to/artist/album/title1.mp3"}},
				TestCase {0, 7, {"/path/to/artist/album/title1.mp3"}},
				TestCase {7, 50, {"/path/to/artist/album/title2.mp3"}},
				TestCase {0, 50, {"/path/to/artist/album/title1.mp3", "/path/to/artist/album/title2.mp3"}},
			};

			for(const auto& testCase: testCases)
			{
				const auto playlist =
					std::make_shared<SmartPlaylistByListeningDate>(0, testCase.from, testCase.to,
					                                               false, -1, fileSystem);

				auto playlistCreator = std::make_shared<PlaylistCreatorMock>();
				auto manager = SmartPlaylistManager(playlistCreator.get(), fileSystem);
				manager.insertPlaylist(playlist);
				manager.selectPlaylist(Spid {playlist->id()});

				auto paths = QStringList {};
				Util::Algorithm::transform(playlistCreator->tracks(), paths, [](const auto& track) {
					return track.filepath();
				});

				QVERIFY(paths == testCase.expectedPaths);
			}
		}
};

QTEST_GUILESS_MAIN(ListeningDateTest)

#include "ListeningDateTest.moc"
