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

#include "PlaylistTestUtils.h"

#include "Common/SayonaraTest.h"
#include "Common/PlayManagerMock.h"

#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	::Playlist::Playlist* createPlaylist(Playlist::Mode mode)
	{
		SetSetting(Set::PL_Mode, mode);

		const auto tracks = Test::Playlist::createTrackList(0, 10);
		QList<int> indexes;
		auto playManager = new PlayManagerMock();
		auto* pl = new Playlist::Playlist(1, "Hallo", playManager);

		pl->createPlaylist(tracks);

		return pl;
	}
}

class RepeatTest :
	public Test::Base
{
	Q_OBJECT

	public:
		RepeatTest() :
			Test::Base("RepeatTest") {}

	private slots:
		void noRepeatAllTest();
		void repeatAllTest();
		void repeatOneTest();
};

void RepeatTest::noRepeatAllTest()
{
	Playlist::Mode mode;
	mode.setRepAll(Playlist::Mode::State::Off);

	auto* playlist = createPlaylist(mode);
	for(auto i = 0; i < Playlist::count(*playlist); i++)
	{
		playlist->next();
		QVERIFY(playlist->currentTrackIndex() >= 0);
	}

	playlist->next();
	QVERIFY(playlist->currentTrackIndex() < 0);
}

void RepeatTest::repeatAllTest()
{
	Playlist::Mode mode;
	mode.setRepAll(Playlist::Mode::State::On);

	auto* playlist = createPlaylist(mode);
	for(auto i = 0; i < Playlist::count(*playlist) * 3; i++)
	{
		playlist->next();
		QVERIFY(playlist->currentTrackIndex() >= 0);
	}
}

void RepeatTest::repeatOneTest()
{
	Playlist::Mode mode;
	mode.setRep1(Playlist::Mode::State::On);

	auto* playlist = createPlaylist(mode);

	playlist->changeTrack(2);
	QVERIFY(playlist->currentTrackIndex() == 2);
	for(auto i = 0; i < 10; i++)
	{
		playlist->next();
		QVERIFY(playlist->currentTrackIndex() == 2);
	}

	playlist->changeTrack(5);
	QVERIFY(playlist->currentTrackIndex() == 5);
	for(auto i = 0; i < 10; i++)
	{
		playlist->next();
		QVERIFY(playlist->currentTrackIndex() == 5);
	}
}

QTEST_GUILESS_MAIN(RepeatTest)

#include "RepeatTest.moc"
