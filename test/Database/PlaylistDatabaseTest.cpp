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

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
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

}

class PlaylistDatabaseTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistDatabaseTest() :
			Test::Base("PlaylistDatabaseTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static, *-function-cognitive-complexity)
		[[maybe_unused]] void testCreateAndDeletePlaylist()
		{
			auto* db = DB::Connector::instance();
			auto* playlistDb = db->playlistConnector();

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
				auto id = playlistDb->createPlaylist(testCase.playlistName, testCase.temporary, false);
				const auto playlist = playlistDb->getPlaylistById(id, false);

				QVERIFY(id >= 0);
				QVERIFY(playlist.tracks().count() == 0);
				QVERIFY(playlist.id() == id);
				QVERIFY(playlist.isTemporary() == testCase.temporary);
				QVERIFY(playlist.name() == testCase.playlistName);

				playlistDb->deletePlaylist(id);
				QVERIFY(playlistDb->getPlaylistById(id, false).id() < 0);
			}
		}

		[[maybe_unused]] void testRadioTrackInsertion() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto* db = DB::Connector::instance();
			auto* playlistDb = db->playlistConnector();

			constexpr const auto* url = "https://url.mp3";
			constexpr const auto* name = "My Radio Station";
			const auto track = createRadioTrack(url, name);

			const auto id = playlistDb->createPlaylist("my playlist", false, false);
			const auto success = playlistDb->insertTrackIntoPlaylist(track, id, 0);
			QVERIFY(success);

			const auto playlist = playlistDb->getPlaylistById(id, true);
			QVERIFY(playlist.tracks().count() == 1);

			playlistDb->deletePlaylist(id);
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
				auto* db = DB::Connector::instance();
				auto* playlistDb = db->playlistConnector();

				const auto id = playlistDb->createPlaylist(testCase.playlistName, true, false);
				playlistDb->insertTrackIntoPlaylist(testCase.track, id, 0);

				auto fetchedPlaylist = playlistDb->getPlaylistById(id, true);
				const auto fetchedTrack = fetchedPlaylist.tracks()[0];
				QVERIFY(fetchedTrack.radioMode() == testCase.expectedRadioMode);

				playlistDb->deletePlaylist(id);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testMixedPlaylistHasCorrectTracks()
		{
			const auto tracks = std::array {
				createRadioTrack("https://stream.com", "some name"),
				createPodcastTrack("podcast", "some artist"),
				createNonLibraryTrack("ar", "album", "ttle")
			};

			auto* db = DB::Connector::instance();
			auto* playlistDb = db->playlistConnector();

			const auto id = playlistDb->createPlaylist("Some palylist", true, false);
			int i = 0;
			for(const auto& track: tracks)
			{
				playlistDb->insertTrackIntoPlaylist(track, id, i);
				i++;
			}

			const auto playlist = playlistDb->getPlaylistById(id, true);
			const auto fetchedTracks = playlist.tracks();

			QVERIFY(fetchedTracks[0] == tracks[0]);
			QVERIFY(fetchedTracks[1] == tracks[1]);
			QVERIFY(fetchedTracks[2] == tracks[2]);
		}
};

QTEST_GUILESS_MAIN(PlaylistDatabaseTest)

#include "PlaylistDatabaseTest.moc"
