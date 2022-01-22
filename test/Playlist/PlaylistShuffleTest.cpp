/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "SayonaraTest.h"
#include "PlaylistTestUtils.h"
#include "PlayManagerMock.h"

#include "Components/Playlist/Playlist.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	::Playlist::Playlist* createShufflePlaylist(int numTracks)
	{
		auto tracks = Test::Playlist::createTrackList(0, numTracks);

		QList<int> indexes;
		auto playManager = new PlayManagerMock();
		auto* pl = new Playlist::Playlist(1, "Hallo", playManager);

		SetSetting(Set::PL_StartPlaying, false);

		Playlist::Mode mode;
		mode.setShuffle(Playlist::Mode::State::On);
		pl->setMode(mode);
		pl->createPlaylist(tracks);

		return pl;
	}
}

class PlaylistShuffleTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistShuffleTest() :
			Test::Base("PlaylistShuffleTest") {}

	private slots:
		void testShuffleFirstTrack();
		void testShuffleOneTrack();
		void testUntilEndOfTracks();
		void testWithRepAll();
		void testWithoutRepAll();
		void testBackward();

	private:
		std::pair<Playlist::Playlist*, QList<int>> prepareRepeatTest(bool withRepeatAll);
};

void PlaylistShuffleTest::testShuffleFirstTrack()
{
	auto* pl = createShufflePlaylist(10);
	pl->play();

	const auto currentIndex = pl->currentTrackIndex();
	QVERIFY(currentIndex > 0);
}

void PlaylistShuffleTest::testShuffleOneTrack()
{
	auto* pl = createShufflePlaylist(1);
	pl->play();

	const auto currentIndex = pl->currentTrackIndex();
	QVERIFY(currentIndex == 0);
}

void PlaylistShuffleTest::testUntilEndOfTracks()
{
	auto* pl = createShufflePlaylist(10);
	const auto& tracks = pl->tracks();

	QList<int> playedTracks;
	QList<int> incrementedTracks;
	for(auto i = 0; i < tracks.count(); i++)
	{
		pl->next();
		playedTracks << pl->currentTrackIndex();
	}
	QVERIFY(playedTracks.count() == tracks.count());
	QVERIFY(pl->currentTrackIndex() != -1);

	for(auto i = 0; i < tracks.count(); i++)
	{
		incrementedTracks << i;
	}
	QVERIFY(playedTracks != incrementedTracks);
}

std::pair<Playlist::Playlist*, QList<int>> PlaylistShuffleTest::prepareRepeatTest(bool withRepeateAll)
{
	auto tracks = Test::Playlist::createTrackList(0, 10);
	auto* playlist = new Playlist::Playlist(1, "Hallo", new PlayManagerMock());

	Playlist::Mode mode;
	mode.setShuffle(Playlist::Mode::State::On);
	const auto repeatAllState = withRepeateAll
	                            ? Playlist::Mode::State::On
	                            : Playlist::Mode::State::Off;
	mode.setRepAll(repeatAllState);

	playlist->setMode(mode);
	playlist->createPlaylist(tracks);

	QList<int> playedTracks;
	for(auto i = 0; i < tracks.count(); i++)
	{
		playlist->next();
		playedTracks << playlist->currentTrackIndex();
	}

	return std::make_pair(playlist, playedTracks);
}

void PlaylistShuffleTest::testWithRepAll()
{
	auto* playlist = prepareRepeatTest(true).first;
	for(int i = 0; i < playlist->count() * 3; i++)
	{
		playlist->next();
		QVERIFY(playlist->currentTrackIndex() >= 0);
	}
}

void PlaylistShuffleTest::testWithoutRepAll()
{
	auto* playlist = prepareRepeatTest(false).first;
	playlist->next();
	QVERIFY(playlist->currentTrackIndex() == -1);
}

void PlaylistShuffleTest::testBackward()
{
	auto[playlist, playedTracks] = prepareRepeatTest(false);
	const auto count = playedTracks.count();
	QVERIFY(playlist->currentTrackIndex() == playedTracks.last());

	for(int i = 1; i < count; i++)
	{
		playlist->bwd();
		const auto currentTrack = playlist->currentTrackIndex();
		const auto expectedTrack = playedTracks[count - i - 1];
		QVERIFY(currentTrack == expectedTrack);
	}
}

QTEST_GUILESS_MAIN(PlaylistShuffleTest)

#include "PlaylistShuffleTest.moc"
