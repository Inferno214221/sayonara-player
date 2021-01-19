#include "SayonaraTest.h"
#include "TestPlayManager.h"
#include "Playlist/PlaylistTestUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Components/Playlist/Playlist.h"
#include "Database/Connector.h"

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
		m_playManager{PlayManagerProvider::instance()->playManager()}
	{}

private:
	PlayManager* m_playManager;

private slots:
	void jump_test();
	void shuffleTest();
	void modifyTest();
	void insertTest();
};


void PlaylistTest::jump_test()
{
	bool success;
	MetaData md;
	MetaDataList v_md = Test::Playlist::createTrackList(0, 100);

	PL* pl = new PL(1, "Hallo", m_playManager);
	success = pl->currentTrack(md);
	QVERIFY(pl->changeTrack(0) == false);
	QVERIFY(pl->index() == 1);
	QVERIFY(pl->runningTime() == 0);
	QVERIFY(pl->currentTrackIndex() == -1);
	QVERIFY(success == false);

	pl->createPlaylist(v_md);
	success = pl->currentTrack(md);
	QVERIFY(pl->tracks().size() == 100);
	QVERIFY(pl->currentTrackIndex() == -1);
	QVERIFY(success == false);

	success = pl->changeTrack(40);
	QVERIFY(success == true);

	success = pl->currentTrack(md);
	QVERIFY(pl->currentTrackIndex() == 40);
	QVERIFY(success == true);
	QVERIFY(md.id() == 40);

	pl->fwd();
	success = pl->currentTrack(md);
	QVERIFY(pl->currentTrackIndex() == 41);
	QVERIFY(success == true);
	QVERIFY(md.id() == 41);

	pl->stop();
	success = pl->currentTrack(md);
	QVERIFY(pl->currentTrackIndex() == -1);
	QVERIFY(success == false);
}

void PlaylistTest::shuffleTest()
{
	MetaData md;
	MetaDataList v_md = Test::Playlist::createTrackList(0, 100);

	QList<int> indexes;
	PL* pl = new PL(1, "Hallo", m_playManager);

	Playlist::Mode mode;
	mode.setShuffle(Playlist::Mode::State::On);
	mode.setRepAll(Playlist::Mode::State::On);

	QVERIFY(Playlist::Mode::isActiveAndEnabled(mode.shuffle()));
	QVERIFY(Playlist::Mode::isActiveAndEnabled(mode.repAll()));

	pl->setMode(mode);
	bool b = pl->changeTrack(0);
	QVERIFY(b == false);

	pl->createPlaylist(v_md);
	b = pl->changeTrack(0);
	indexes << 0;
	QVERIFY(b == true);

	for(int i=1; i<v_md.count(); i++)
	{
		pl->next();

		int curIndex = pl->currentTrackIndex();
		QVERIFY(indexes.contains(curIndex) == false);
		indexes << curIndex;
	}

	QVERIFY(indexes.count() == v_md.count());

	QList<int> indexes_cpy = indexes;

	std::sort(indexes.begin(), indexes.end());
	QVERIFY(indexes_cpy != indexes);
}

void PlaylistTest::modifyTest()
{
	MetaDataList v_md = Test::Playlist::createTrackList(0, 100);
	int curIndex;

	auto pl = std::make_shared<PL>(1, "Hallo", m_playManager);
	pl->createPlaylist(v_md);
	const MetaDataList& plTracks = pl->tracks();

	QList<UniqueId> pl_uids = plTracks.unique_ids();

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
		curIndex = pl->currentTrackIndex();
		QVERIFY(curIndex == 46);
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
		curIndex = pl->currentTrackIndex();
		QVERIFY(curIndex == 11);
	}

	{ // move current track
		indexes.clear();
		{
			indexes << 11;		// new 20 - indexes.size() = 18
			indexes << 12;		// new 19
		}

		pl->moveTracks(indexes, 20);
		curIndex = pl->currentTrackIndex();
		QVERIFY(curIndex == 18);
	}

	{ // check if uids haven't changed
		QList<UniqueId> pl_uids_after = plTracks.unique_ids();
		QVERIFY(pl_uids != pl_uids_after);

		std::sort(pl_uids.begin(), pl_uids.end());
		std::sort(pl_uids_after.begin(), pl_uids_after.end());
		QVERIFY(pl_uids == pl_uids_after);
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
		curIndex = pl->currentTrackIndex();
		QVERIFY(curIndex == 14);
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
	curIndex = pl->currentTrackIndex();
	QVERIFY(curIndex == -1);
}

void PlaylistTest::insertTest()
{
	auto pl = std::make_shared<PL>(1, "Hallo", m_playManager);
	pl->createPlaylist(MetaDataList());

	{
		MetaDataList tracks = Test::Playlist::createTrackList(0, 3);
		pl->insertTracks(tracks, 20);

		MetaDataList tracks_pl = pl->tracks();
		QVERIFY(pl->count() == 3);
		QVERIFY(pl->count() == tracks_pl.count());

		for(int i=0; i<tracks_pl.count(); i++)
		{
			QVERIFY(tracks_pl[i].id() == i);
		}

		pl->clear();
		QVERIFY(pl->count() == 0);
	}


	{
		MetaDataList tracks = Test::Playlist::createTrackList(0, 3);
		pl->insertTracks(tracks, -1);

		MetaDataList tracks_pl = pl->tracks();
		QVERIFY(pl->count() == 3);
		QVERIFY(pl->count() == tracks_pl.count());

		for(int i=0; i<tracks_pl.count(); i++)
		{
			QVERIFY(tracks_pl[i].id() == i);
		}
	}

	{
		MetaDataList tracks = Test::Playlist::createTrackList(3, 4);
		pl->insertTracks(tracks, -1);

		MetaDataList tracks_pl = pl->tracks();
		QVERIFY(pl->count() == 4);
		QVERIFY(pl->count() == tracks_pl.count());

		QVERIFY(tracks_pl.first().id() == 3);
	}

	{
		MetaDataList tracks = Test::Playlist::createTrackList(4, 5);
		pl->insertTracks(tracks, 3);

		MetaDataList tracks_pl = pl->tracks();
		QVERIFY(pl->count() == 5);
		QVERIFY(pl->count() == tracks_pl.count());

		QVERIFY(tracks_pl[3].id() == 4);
	}

	{
		MetaDataList tracks = Test::Playlist::createTrackList(5, 6);
		pl->insertTracks(tracks, pl->count());

		MetaDataList tracks_pl = pl->tracks();
		QVERIFY(pl->count() == 6);
		QVERIFY(pl->count() == tracks_pl.count());

		QVERIFY(tracks_pl.last().id() == 5);
	}
}

QTEST_GUILESS_MAIN(PlaylistTest)

#include "PlaylistTest.moc"

