#include "PlaylistTestUtils.h"

#include "test/Common/SayonaraTest.h"
#include "test/Common/PlayManagerMock.h"

#include "Components/Playlist/Playlist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Utils.h"

using PL = Playlist::Playlist;

namespace
{
	inline std::shared_ptr<Playlist::Playlist>
	createPlaylist(int index, int min, int max, const QString& name, PlayManager* playManager)
	{
		const auto tracks = Test::Playlist::createTrackList(min, max);
		auto playlist = std::make_shared<PL>(index, name, playManager);
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

	private slots:
		void jumpTest();
		void modifyTest();
		void insertTest();
		void trackIndexWithoutDisabledTest();
};

void PlaylistTest::jumpTest()
{
	bool success;
	MetaData track;
	MetaDataList tracks = Test::Playlist::createTrackList(0, 100);

	auto* playlist = new PL(1, "Hallo", m_playManager);
	success = playlist->currentTrack(track);
	QVERIFY(playlist->changeTrack(0) == false);
	QVERIFY(playlist->index() == 1);
	QVERIFY(playlist->runningTime() == 0);
	QVERIFY(playlist->currentTrackIndex() == -1);
	QVERIFY(success == false);

	playlist->createPlaylist(tracks);
	success = playlist->currentTrack(track);
	QVERIFY(playlist->tracks().size() == 100);
	QVERIFY(playlist->currentTrackIndex() == -1);
	QVERIFY(success == false);

	success = playlist->changeTrack(40);
	QVERIFY(success == true);

	success = playlist->currentTrack(track);
	QVERIFY(playlist->currentTrackIndex() == 40);
	QVERIFY(success == true);
	QVERIFY(track.id() == 40);

	playlist->fwd();
	success = playlist->currentTrack(track);
	QVERIFY(playlist->currentTrackIndex() == 41);
	QVERIFY(success == true);
	QVERIFY(track.id() == 41);

	playlist->stop();
	success = playlist->currentTrack(track);
	QVERIFY(playlist->currentTrackIndex() == -1);
	QVERIFY(success == false);
}

void PlaylistTest::modifyTest()
{
	auto tracks = Test::Playlist::createTrackList(0, 100);
	int currentIndex;

	auto pl = std::make_shared<PL>(1, "Hallo", m_playManager);
	pl->createPlaylist(tracks);
	const auto& plTracks = pl->tracks();

	auto uniqueIds = plTracks.unique_ids();

	pl->changeTrack(50);
	QVERIFY(pl->currentTrackIndex() == 50);

	IndexSet indexes;
	{ // move indices before cur track
		{
			indexes << 1;
			indexes << 2;
			indexes << 3;
			indexes << 4;
		}

		pl->moveTracks(indexes, 75);
		currentIndex = pl->currentTrackIndex();
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

		pl->moveTracks(indexes, 10);
		currentIndex = pl->currentTrackIndex();
		QVERIFY(currentIndex == 11);
	}

	{ // move current track
		indexes.clear();
		{
			indexes << 11;        // new 20 - indexes.size() = 18
			indexes << 12;        // new 19
		}

		pl->moveTracks(indexes, 20);
		currentIndex = pl->currentTrackIndex();
		QVERIFY(currentIndex == 18);
	}

	{ // check if uids haven't changed
		auto currentUniqueIds = plTracks.unique_ids();
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

		pl->removeTracks(indexes);
		currentIndex = pl->currentTrackIndex();
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

	pl->removeTracks(indexes);
	currentIndex = pl->currentTrackIndex();
	QVERIFY(currentIndex == -1);
}

void PlaylistTest::insertTest()
{
	auto pl = std::make_shared<PL>(1, "Hallo", m_playManager);
	pl->createPlaylist(MetaDataList());

	{
		const auto tracks = Test::Playlist::createTrackList(0, 3);
		pl->insertTracks(tracks, 20);

		const auto playlistTracks = pl->tracks();
		QVERIFY(pl->count() == 3);
		QVERIFY(pl->count() == playlistTracks.count());

		for(int i = 0; i < playlistTracks.count(); i++)
		{
			QVERIFY(playlistTracks[i].id() == i);
		}

		pl->clear();
		QVERIFY(pl->count() == 0);
	}

	{
		const auto tracks = Test::Playlist::createTrackList(0, 3);
		pl->insertTracks(tracks, -1);

		const auto playlistTracks = pl->tracks();
		QVERIFY(pl->count() == 3);
		QVERIFY(pl->count() == playlistTracks.count());

		for(int i = 0; i < playlistTracks.count(); i++)
		{
			QVERIFY(playlistTracks[i].id() == i);
		}
	}

	{
		const auto tracks = Test::Playlist::createTrackList(3, 4);
		pl->insertTracks(tracks, -1);

		const auto playlistTracks = pl->tracks();
		QVERIFY(pl->count() == 4);
		QVERIFY(pl->count() == playlistTracks.count());

		QVERIFY(playlistTracks.first().id() == 3);
	}

	{
		const auto tracks = Test::Playlist::createTrackList(4, 5);
		pl->insertTracks(tracks, 3);

		const auto playlistTracks = pl->tracks();
		QVERIFY(pl->count() == 5);
		QVERIFY(pl->count() == playlistTracks.count());

		QVERIFY(playlistTracks[3].id() == 4);
	}

	{
		const auto tracks = Test::Playlist::createTrackList(5, 6);
		pl->insertTracks(tracks, pl->count());

		const auto playlistTracks = pl->tracks();
		QVERIFY(pl->count() == 6);
		QVERIFY(pl->count() == playlistTracks.count());

		QVERIFY(playlistTracks.last().id() == 5);
	}
}

void PlaylistTest::trackIndexWithoutDisabledTest()
{
	{ // empty playlist
		auto playlist = PL(1, "Hallo", m_playManager);
		QVERIFY(playlist.currentTrackWithoutDisabled() == -1);
	}

	{ // non-active playlist
		auto playlist = createPlaylist(1, 0, 10, "hallo", m_playManager);
		QVERIFY(playlist->currentTrackWithoutDisabled() == -1);
	}

	{ // test all enabled
		auto playlist = createPlaylist(1, 0, 10, "hallo", m_playManager);
		playlist->changeTrack(4);

		QVERIFY(playlist->currentTrackIndex() == 4);
		QVERIFY(playlist->currentTrackWithoutDisabled() == 4);
	}

	{ // test invalid index
		auto playlist = createPlaylist(1, 0, 10, "hallo", m_playManager);

		playlist->changeTrack(-1);
		QVERIFY(playlist->currentTrackWithoutDisabled() == -1);

		playlist->changeTrack(100);
		QVERIFY(playlist->currentTrackWithoutDisabled() == -1);
	}

	{ // all disabled except current index
		const auto currentIndex = 4;
		auto tracks = Test::Playlist::createTrackList(0, 10);
		for(auto& track : tracks)
		{
			track.setDisabled(true);
		}

		tracks[currentIndex].setDisabled(false);

		auto playlist = PL(1, "Hallo", m_playManager);
		playlist.createPlaylist(tracks);
		playlist.changeTrack(currentIndex);
		QVERIFY(playlist.currentTrackWithoutDisabled() == 0);
	}

	{ // all enabled except current index
		const auto currentIndex = 4;
		auto tracks = Test::Playlist::createTrackList(0, 10);

		tracks[currentIndex].setDisabled(true);

		auto playlist = PL(1, "Hallo", m_playManager);
		playlist.createPlaylist(tracks);

		playlist.changeTrack(currentIndex);
		QVERIFY(playlist.currentTrackWithoutDisabled() == -1);
	}

	{ // test some disabled
		auto tracks = Test::Playlist::createTrackList(0, 10);
		tracks[0].setDisabled(true);
		tracks[2].setDisabled(true);
		tracks[4].setDisabled(true);
		tracks[6].setDisabled(true);
		// enabled: 1, 3, 5, 7, 9

		auto playlist = PL(1, "Hallo", m_playManager);
		playlist.createPlaylist(tracks);

		playlist.changeTrack(4);
		QVERIFY(playlist.currentTrackWithoutDisabled() == -1);

		playlist.changeTrack(5);
		QVERIFY(playlist.currentTrackWithoutDisabled() == 2);
	}
}

QTEST_GUILESS_MAIN(PlaylistTest)

#include "PlaylistTest.moc"

