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
		auto track = MetaData {};
		track.setRadioStation(url, name);
		track.changeRadioMode(RadioMode::Station);
		return track;
	}
}

class PlaylistDatabaseTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistDatabaseTest() :
			Test::Base("PlaylistTest") {}

	private slots:

		[[maybe_unused]] void testCreateAndDeletePlaylist() // NOLINT(readability-convert-member-functions-to-static)
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
				auto id = playlistDb->createPlaylist(testCase.playlistName, testCase.temporary);
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

			const auto id = playlistDb->createPlaylist("my playlist", false);
			const auto success = playlistDb->insertTrackIntoPlaylist(track, id, 0);
			QVERIFY(success);

			const auto playlist = playlistDb->getPlaylistById(id, true);
			QVERIFY(playlist.tracks().count() == 1);

			const auto fetchedTrack = playlist.tracks()[0];
			QVERIFY(fetchedTrack.radioMode() == RadioMode::Station);
			QVERIFY(fetchedTrack.radioStation() == url);
			QVERIFY(fetchedTrack.radioStationName() == name);

			playlistDb->deletePlaylist(id);
		}
};

QTEST_GUILESS_MAIN(PlaylistDatabaseTest)

#include "PlaylistDatabaseTest.moc"
