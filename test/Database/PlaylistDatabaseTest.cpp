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

#include "Common/SayonaraTest.h"
#include "Database/Playlist.h"
#include "Database/Connector.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/MetaData/MetaDataList.h"
#include <QSqlQuery>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	struct TestEnv
	{
		~TestEnv()
		{
			const auto playlists = playlistDatabase->getAllPlaylists(PlaylistStoreType::TemporaryAndPermanent, false);
			for(const auto& playlist: playlists)
			{
				playlistDatabase->deletePlaylist(playlist.id());
			}
		}

		DB::Playlist* playlistDatabase {DB::Connector::instance()->playlistConnector()};
	};

	MetaData createRadioTrack(const QString& url, const QString& name)
	{
		auto track = MetaData {url};
		track.setRadioStation(url, name);
		track.changeRadioMode(RadioMode::Station);
		return track;
	}

	MetaData createPodcastTrack(const QString& title, const QString& artist)
	{
		auto track = MetaData {QString("https://podcast%1%2.com").arg(artist).arg(title)};
		track.setTitle(title);
		track.setArtist(artist);
		track.changeRadioMode(RadioMode::Podcast);
		return track;
	}

	MetaData createNonLibraryTrack(const QString& artist, const QString& album, const QString& title)
	{
		const auto filepath = QString("/%1/%2/%3.mp3")
			.arg(artist)
			.arg(album)
			.arg(title);

		auto track = MetaData {filepath};
		track.setArtist(artist);
		track.setAlbum(album);
		track.setTitle(title);

		return track;
	}

	int getEntriesOfOnlineTracks()
	{
		auto* db = DB::Connector::instance();
		auto q = db->runQuery("SELECT COUNT(rowId) FROM OnlineTracks;", "Cannot get online track count");

		return q.next()
		       ? q.value(0).toInt()
		       : -1;
	}

	int getEntriesOfPlaylistToTracks()
	{
		auto* db = DB::Connector::instance();
		auto q = db->runQuery("SELECT COUNT(rowId) FROM playlistToTracks;",
		                      "Cannot get playlistToTracks track count");

		return q.next()
		       ? q.value(0).toInt()
		       : -1;
	}
}

class PlaylistDatabaseTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistDatabaseTest() :
			Test::Base("PlaylistDatabaseTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testDependentDatabasesAreDeleted()
		{
			auto env = TestEnv();
			const auto tracks = std::array {
				createRadioTrack("https://stream.com", "some name"),
				createPodcastTrack("podcast", "some artist"),
				createNonLibraryTrack("ar", "album", "ttle")
			};

			const auto id = env.playlistDatabase->createPlaylist("playlist", true, false);
			int i = 0;
			for(const auto& track: tracks)
			{
				env.playlistDatabase->insertTrackIntoPlaylist(track, id, i);
				i++;
			}

			QCOMPARE(getEntriesOfOnlineTracks(), 2);
			QCOMPARE(getEntriesOfPlaylistToTracks(), 3);

			env.playlistDatabase->deletePlaylist(id);

			QCOMPARE(getEntriesOfOnlineTracks(), 0);
			QCOMPARE(getEntriesOfPlaylistToTracks(), 0);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static, *-function-cognitive-complexity)
		[[maybe_unused]] void testCreateAndDeletePlaylist()
		{
			auto env = TestEnv();

			struct TestCase
			{
				QString playlistName;
				bool temporary;
				int expectedId;
			};

			const auto testCases = std::array {
				TestCase {"Temporary Playlist", true, 1},
				TestCase {"Non-Temporary Playlist", false, 2}
			};

			for(const auto& testCase: testCases)
			{
				auto id = env.playlistDatabase->createPlaylist(testCase.playlistName, testCase.temporary, false);
				const auto playlist = env.playlistDatabase->getPlaylistById(id, false);

				QVERIFY(id >= 0);
				QVERIFY(playlist.tracks().count() == 0);
				QVERIFY(playlist.id() == id);
				QVERIFY(playlist.isTemporary() == testCase.temporary);
				QVERIFY(playlist.name() == testCase.playlistName);
			}
		}

		[[maybe_unused]] void testRadioTrackInsertion() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto env = TestEnv();

			constexpr const auto* url = "https://url.mp3";
			constexpr const auto* name = "My Radio Station";
			const auto track = createRadioTrack(url, name);

			const auto id = env.playlistDatabase->createPlaylist("my playlist", false, false);
			const auto success = env.playlistDatabase->insertTrackIntoPlaylist(track, id, 0);
			QVERIFY(success);

			const auto playlist = env.playlistDatabase->getPlaylistById(id, true);
			QVERIFY(playlist.tracks().count() == 1);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testRadioModeIsLoadedCorrectly()
		{
			struct TestCase
			{
				QString playlistName;
				MetaData track;
				RadioMode expectedRadioMode;
			};

			const auto testCases = std::array {
				TestCase {"StreamPlaylist", createRadioTrack("https://stream.com", "stream"), RadioMode::Station},
				TestCase {"PodcastPlaylist", createPodcastTrack("https://podcast.com", "stream"), RadioMode::Podcast},
				TestCase {"StandardPlaylist", createNonLibraryTrack("ar", "al", "t"), RadioMode::Off}
			};

			for(const auto& testCase: testCases)
			{
				TestEnv env;

				const auto id = env.playlistDatabase->createPlaylist(testCase.playlistName, true, false);
				env.playlistDatabase->insertTrackIntoPlaylist(testCase.track, id, 0);

				auto fetchedPlaylist = env.playlistDatabase->getPlaylistById(id, true);
				const auto fetchedTrack = fetchedPlaylist.tracks()[0];
				QVERIFY(fetchedTrack.radioMode() == testCase.expectedRadioMode);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testMixedPlaylistHasCorrectTracks()
		{
			TestEnv env;
			const auto tracks = std::array {
				createRadioTrack("https://stream.com", "some name"),
				createPodcastTrack("podcast", "some artist"),
				createNonLibraryTrack("ar", "album", "ttle")
			};

			const auto id = env.playlistDatabase->createPlaylist("Some palylist", true, false);
			int i = 0;
			for(const auto& track: tracks)
			{
				env.playlistDatabase->insertTrackIntoPlaylist(track, id, i);
				i++;
			}

			const auto playlist = env.playlistDatabase->getPlaylistById(id, true);
			const auto fetchedTracks = playlist.tracks();

			QVERIFY(fetchedTracks[0] == tracks[0]);
			QVERIFY(fetchedTracks[1] == tracks[1]);
			QVERIFY(fetchedTracks[2] == tracks[2]);
		}
};

QTEST_GUILESS_MAIN(PlaylistDatabaseTest)

#include "PlaylistDatabaseTest.moc"
