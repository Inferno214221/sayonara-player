#include "SayonaraTest.h"
#include "TestPlayManager.h"
#include "Playlist/PlaylistTestUtils.h"

#include "Components/Playlist/Playlist.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Utils.h"

using PL=Playlist::Playlist;

class PlaylistTest :
	public Test::Base
{
	Q_OBJECT

public:
	PlaylistTest() :
		Test::Base("PlaylistTest"),
		m_playManager{new TestPlayManager(this)}
	{}

	~PlaylistTest()
	{
		delete m_playManager;
	}

private:
	PlayManager* m_playManager;

private slots:
	void jumpTest();
	void shuffleTest();
	void modifyTest();
	void insertTest();
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

void PlaylistTest::shuffleTest()
{
	MetaData track;
	auto tracks = Test::Playlist::createTrackList(0, 100);

	QList<int> indexes;
	auto* pl = new PL(1, "Hallo", m_playManager);

	Playlist::Mode mode;
	mode.setShuffle(Playlist::Mode::State::On);
	mode.setRepAll(Playlist::Mode::State::On);

	QVERIFY(Playlist::Mode::isActiveAndEnabled(mode.shuffle()));
	QVERIFY(Playlist::Mode::isActiveAndEnabled(mode.repAll()));

	pl->setMode(mode);
	bool b = pl->changeTrack(0);
	QVERIFY(b == false);

	pl->createPlaylist(tracks);
	b = pl->changeTrack(0);
	indexes << 0;
	QVERIFY(b == true);

	for(int i=1; i < tracks.count(); i++)
	{
		pl->next();

		int curIndex = pl->currentTrackIndex();
		QVERIFY(indexes.contains(curIndex) == false);
		indexes << curIndex;
	}

	QVERIFY(indexes.count() == tracks.count());

	QList<int> oldIndexes = indexes;

	std::sort(indexes.begin(), indexes.end());
	QVERIFY(oldIndexes != indexes);
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
			indexes << 65;		// new 12
			indexes << 32;		// new 10
			indexes << 46;		// new 11
			indexes << 6;		// new 9
		}

		pl->moveTracks(indexes, 10);
		currentIndex = pl->currentTrackIndex();
		QVERIFY(currentIndex == 11);
	}

	{ // move current track
		indexes.clear();
		{
			indexes << 11;		// new 20 - indexes.size() = 18
			indexes << 12;		// new 19
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

		for(int i=0; i < playlistTracks.count(); i++)
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

		for(int i=0; i < playlistTracks.count(); i++)
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

QTEST_GUILESS_MAIN(PlaylistTest)

#include "PlaylistTest.moc"

