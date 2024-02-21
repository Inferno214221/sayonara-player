/* PlaylistTest.cpp
 *
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

#include "test/Common/SayonaraTest.h"
#include "test/Common/PlayManagerMock.h"
#include "test/Common/FileSystemMock.h"

#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Utils.h"

using PL = Playlist::Playlist;

namespace
{
	using PlaylistPtr = std::shared_ptr<Playlist::Playlist>;

	inline std::shared_ptr<Playlist::Playlist>
	createPlaylist(int index, int min, int max, const QString& name, PlayManager* playManager)
	{
		auto fileSystem = std::make_shared<Test::AllFilesAvailableFileSystem>();
		const auto tracks = Test::Playlist::createTrackList(min, max);

		auto playlist = std::make_shared<PL>(index, name, playManager, fileSystem);
		playlist->createPlaylist(tracks);

		return playlist;
	}
}

class PlaylistTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistTest() :
			Test::Base("PlaylistTest"),
			m_playManager {new PlayManagerMock()} {}

		~PlaylistTest() override
		{
			delete m_playManager;
		}

	private:
		PlayManager* m_playManager;
		Util::FileSystemPtr m_fileSystem = std::make_shared<Test::AllFilesAvailableFileSystem>();

	private slots:
		void jumpTest();
		void modifyTest();
		void insertTest();
		void trackIndexWithoutDisabledTest();
		void uniqueIdTest();
};

void PlaylistTest::jumpTest()
{
	MetaDataList tracks = Test::Playlist::createTrackList(0, 100);

	auto playlist = Playlist::Playlist(1, "Hallo", m_playManager, m_fileSystem);
	QVERIFY(playlist.changeTrack(0) == false);
	QVERIFY(playlist.index() == 1);
	QVERIFY(runningTime(playlist) == 0);
	QVERIFY(playlist.currentTrackIndex() == -1);
	QVERIFY(playlist.tracks().isEmpty());

	playlist.createPlaylist(tracks);
	{
		QVERIFY(playlist.tracks().size() == 100);
		QVERIFY(playlist.currentTrackIndex() == -1);
		QVERIFY(playlist.tracks().count() == tracks.count());
	}

	const auto success = playlist.changeTrack(40);
	{
		QVERIFY(success);
		QVERIFY(playlist.currentTrackIndex() == 40);
		const auto playlistTracks = playlist.tracks();
		QVERIFY(tracks[40].id() == 40);
	}

	playlist.fwd();
	{
		QVERIFY(playlist.currentTrackIndex() == 41);
	}

	playlist.stop();
	{
		QVERIFY(playlist.currentTrackIndex() == -1);
	}
}

void PlaylistTest::modifyTest()
{
	auto tracks = Test::Playlist::createTrackList(0, 100);
	auto playlist = PL(1, "Hallo", m_playManager, m_fileSystem);
	playlist.createPlaylist(tracks);
	auto currentIndex = playlist.currentTrackIndex();
	const auto& plTracks = playlist.tracks();

	auto uniqueIds = Util::uniqueIds(plTracks);

	playlist.changeTrack(50);
	QVERIFY(playlist.currentTrackIndex() == 50);

	IndexSet indexes;
	{ // move indices before cur track
		{
			indexes << 1;
			indexes << 2;
			indexes << 3;
			indexes << 4;
		}

		Playlist::moveTracks(playlist, indexes, 75, Playlist::Reason::Undefined);
		currentIndex = playlist.currentTrackIndex();
		QVERIFY(currentIndex == 46);
	}

	{ // move before, after and with current track
		indexes.clear();
		{
			indexes << 65;        // new 12
			indexes << 32;        // new 10
			indexes << 46;        // new 11
			indexes << 6;        // new 9
		}

		Playlist::moveTracks(playlist, indexes, 10, Playlist::Reason::Undefined);
		currentIndex = playlist.currentTrackIndex();
		QVERIFY(currentIndex == 11);
	}

	{ // move current track
		indexes.clear();
		{
			indexes << 11;        // new 20 - indexes.size() = 18
			indexes << 12;        // new 19
		}

		Playlist::moveTracks(playlist, indexes, 20, Playlist::Reason::Undefined);
		currentIndex = playlist.currentTrackIndex();
		QVERIFY(currentIndex == 18);
	}

	{ // check if uids haven't changed
		auto currentUniqueIds = Util::uniqueIds(plTracks);
		QVERIFY(uniqueIds != currentUniqueIds);

		std::sort(uniqueIds.begin(), uniqueIds.end());
		std::sort(currentUniqueIds.begin(), currentUniqueIds.end());
		QVERIFY(uniqueIds == currentUniqueIds);
	}

	{ // remove a few tracks
		indexes.clear();
		{
			indexes << 1;
			indexes << 2;
			indexes << 3;
			indexes << 4;
		}

		Playlist::removeTracks(playlist, indexes, Playlist::Reason::Undefined);
		currentIndex = playlist.currentTrackIndex();
		QVERIFY(currentIndex == 14);
	}

	{ // finally, remove current track
		indexes.clear();
		{
			indexes << 4;
			indexes << 6;
			indexes << 9;
			indexes << 14;
			indexes << 19;
		}
	}

	Playlist::removeTracks(playlist, indexes, Playlist::Reason::Undefined);
	currentIndex = playlist.currentTrackIndex();
	QVERIFY(currentIndex == -1);
}

void PlaylistTest::insertTest()
{
	auto playlist = PL(1, "Hallo", m_playManager, m_fileSystem);
	playlist.createPlaylist(MetaDataList());

	{
		const auto tracks = Test::Playlist::createTrackList(0, 3);
		Playlist::insertTracks(playlist, tracks, 20, Playlist::Reason::Undefined);

		const auto playlistTracks = playlist.tracks();
		QVERIFY(Playlist::count(playlist) == 3);
		QVERIFY(Playlist::count(playlist) == playlistTracks.count());

		for(int i = 0; i < playlistTracks.count(); i++)
		{
			QVERIFY(playlistTracks[i].id() == i);
		}

		Playlist::clear(playlist, Playlist::Reason::Undefined);
		QVERIFY(Playlist::count(playlist) == 0);
	}

	{
		const auto tracks = Test::Playlist::createTrackList(0, 3);
		Playlist::insertTracks(playlist, tracks, -1, Playlist::Reason::Undefined);

		const auto playlistTracks = playlist.tracks();
		QVERIFY(Playlist::count(playlist) == 3);
		QVERIFY(Playlist::count(playlist) == playlistTracks.count());

		for(int i = 0; i < playlistTracks.count(); i++)
		{
			QVERIFY(playlistTracks[i].id() == i);
		}
	}

	{
		const auto tracks = Test::Playlist::createTrackList(3, 4);
		Playlist::insertTracks(playlist, tracks, -1, Playlist::Reason::Undefined);

		const auto playlistTracks = playlist.tracks();
		QVERIFY(Playlist::count(playlist) == 4);
		QVERIFY(Playlist::count(playlist) == playlistTracks.count());

		QVERIFY(playlistTracks[0].id() == 3);
	}

	{
		const auto tracks = Test::Playlist::createTrackList(4, 5);
		Playlist::insertTracks(playlist, tracks, 3, Playlist::Reason::Undefined);

		const auto playlistTracks = playlist.tracks();
		QVERIFY(Playlist::count(playlist) == 5);
		QVERIFY(Playlist::count(playlist) == playlistTracks.count());

		QVERIFY(playlistTracks[3].id() == 4);
	}

	{
		const auto tracks = Test::Playlist::createTrackList(5, 6);
		Playlist::insertTracks(playlist, tracks, Playlist::count(playlist), Playlist::Reason::Undefined);

		const auto playlistTracks = playlist.tracks();
		QVERIFY(Playlist::count(playlist) == 6);
		QVERIFY(Playlist::count(playlist) == playlistTracks.count());

		const auto lastIndex = playlistTracks.count() - 1;
		QVERIFY(playlistTracks[lastIndex].id() == 5);
	}
}

void PlaylistTest::trackIndexWithoutDisabledTest()
{
	{ // empty playlist
		auto playlist = PL(1, "Hallo", m_playManager, m_fileSystem);
		QVERIFY(Playlist::currentTrackWithoutDisabled(playlist) == -1);
	}

	{ // non-active playlist
		auto playlist = createPlaylist(1, 0, 10, "hallo", m_playManager);
		QVERIFY(Playlist::currentTrackWithoutDisabled(*playlist) == -1);
	}

	{ // test all enabled
		auto playlist = createPlaylist(1, 0, 10, "hallo", m_playManager);
		playlist->changeTrack(4);

		QVERIFY(playlist->currentTrackIndex() == 4);
		QVERIFY(Playlist::currentTrackWithoutDisabled(*playlist) == 4);
	}

	{ // test invalid index
		auto playlist = createPlaylist(1, 0, 10, "hallo", m_playManager);

		playlist->changeTrack(-1);
		QVERIFY(Playlist::currentTrackWithoutDisabled(*playlist) == -1);

		playlist->changeTrack(100);
		QVERIFY(Playlist::currentTrackWithoutDisabled(*playlist) == -1);
	}

	{ // all disabled except current index
		const auto currentIndex = 4;
		auto tracks = Test::Playlist::createTrackList(0, 10);
		for(auto& track: tracks)
		{
			track.setDisabled(true);
		}

		tracks[currentIndex].setDisabled(false);

		auto playlist = PL(1, "Hallo", m_playManager, m_fileSystem);
		playlist.createPlaylist(tracks);
		playlist.changeTrack(currentIndex);
		QVERIFY(Playlist::currentTrackWithoutDisabled(playlist) == 0);
	}

	{ // all enabled except current index
		const auto currentIndex = 4;
		auto tracks = Test::Playlist::createTrackList(0, 10);

		tracks[currentIndex].setDisabled(true);

		auto playlist = PL(1, "Hallo", m_playManager, m_fileSystem);
		playlist.createPlaylist(tracks);

		playlist.changeTrack(currentIndex);
		QVERIFY(Playlist::currentTrackWithoutDisabled(playlist) == -1);
	}

	{ // test some disabled
		auto tracks = Test::Playlist::createTrackList(0, 10);
		tracks[0].setDisabled(true);
		tracks[2].setDisabled(true);
		tracks[4].setDisabled(true);
		tracks[6].setDisabled(true);
		// enabled: 1, 3, 5, 7, 9

		auto playlist = PL(1, "Hallo", m_playManager, m_fileSystem);
		playlist.createPlaylist(tracks);

		playlist.changeTrack(4);
		QVERIFY(Playlist::currentTrackWithoutDisabled(playlist) == -1);

		playlist.changeTrack(5);
		QVERIFY(Playlist::currentTrackWithoutDisabled(playlist) == 2);
	}
}

void PlaylistTest::uniqueIdTest()
{
	auto playlist = createPlaylist(1, 0, 10, "hallo", m_playManager);

	auto uniqueIds = QList<UniqueId> {};
	Util::Algorithm::transform(playlist->tracks(), uniqueIds, [](const MetaData& track) {
		return track.uniqueId();
	});

	Playlist::reverse(*playlist, Playlist::Reason::Undefined);

	auto newUniqueIds = QList<UniqueId> {};
	Util::Algorithm::transform(playlist->tracks(), newUniqueIds, [](const auto& track) {
		return track.uniqueId();
	});

	QVERIFY(uniqueIds != newUniqueIds);
	std::reverse(uniqueIds.begin(), uniqueIds.end());

	QVERIFY(uniqueIds == newUniqueIds);
}

QTEST_GUILESS_MAIN(PlaylistTest)

#include "PlaylistTest.moc"

