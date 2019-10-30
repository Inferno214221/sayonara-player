#include <QTest>
#include "Utils/MetaData/MetaDataList.h"
#include "Components/Playlist/Playlist.h"

#include "Utils/Set.h"

using PL=Playlist::Playlist;

class PlaylistTest :
	public QObject
{
	Q_OBJECT

private slots:
	void test();
	void shuffleTest();
	void modifyTest();
};

static MetaDataList create_v_md(int min, int max)
{
	MetaDataList v_md;
	for(int i=min; i<max; i++)
	{
		MetaData md;
		md.set_id(i);

		QString p = QString("https://www.bla.com/path/to/%1.mp3").arg(i);
		md.set_filepath(p);

		md.set_duration_ms(i * 10000);

		v_md << md;
	}

	return v_md;
}

void PlaylistTest::test()
{
	bool success;
	MetaData md;
	MetaDataList v_md = create_v_md(0, 100);

	PL* pl = new PL(1, PlaylistType::Std, "Hallo");
	success = pl->current_track(md);
	QVERIFY(pl->change_track(0) == false);
	QVERIFY(pl->index() == 1);
	QVERIFY(pl->running_time() == 0);
	QVERIFY(pl->current_track_index() == -1);
	QVERIFY(success == false);

	pl->create_playlist(v_md);
	success = pl->current_track(md);
	QVERIFY(pl->tracks().size() == 100);
	QVERIFY(pl->current_track_index() == -1);
	QVERIFY(success == false);

	success = pl->change_track(40);
	QVERIFY(success == true);

	success = pl->current_track(md);
	QVERIFY(pl->current_track_index() == 40);
	QVERIFY(success == true);
	QVERIFY(md.id() == 40);

	pl->fwd();
	success = pl->current_track(md);
	QVERIFY(pl->current_track_index() == 41);
	QVERIFY(success == true);
	QVERIFY(md.id() == 41);

	pl->stop();
	success = pl->current_track(md);
	QVERIFY(pl->current_track_index() == -1);
	QVERIFY(success == false);
}

void PlaylistTest::shuffleTest()
{
	MetaData md;
	MetaDataList v_md = create_v_md(0, 100);

	QList<int> indexes;
	PL* pl = new PL(1, PlaylistType::Std, "Hallo");

	Playlist::Mode mode;
	mode.setShuffle(Playlist::Mode::State::On);
	mode.setRepAll(Playlist::Mode::State::On);

	QVERIFY(Playlist::Mode::isActiveAndEnabled(mode.shuffle()));
	QVERIFY(Playlist::Mode::isActiveAndEnabled(mode.repAll()));

	pl->set_mode(mode);
	bool b = pl->change_track(0);
	QVERIFY(b == false);

	pl->create_playlist(v_md);
	b = pl->change_track(0);
	indexes << 0;
	QVERIFY(b == true);

	for(int i=1; i<v_md.count(); i++)
	{
		pl->next();

		int cur_idx = pl->current_track_index();
		QVERIFY(indexes.contains(cur_idx) == false);
		indexes << cur_idx;
	}

	QVERIFY(indexes.count() == v_md.count());

	QList<int> indexes_cpy = indexes;

	std::sort(indexes.begin(), indexes.end());
	QVERIFY(indexes_cpy != indexes);
}

void PlaylistTest::modifyTest()
{
	MetaDataList v_md = create_v_md(0, 100);
	int cur_idx;

	PL* pl = new PL(1, PlaylistType::Std, "Hallo");
	pl->create_playlist(v_md);
	const MetaDataList& pl_tracks = pl->tracks();

	QList<UniqueId> pl_uids = pl_tracks.unique_ids();

	pl->change_track(50);
	QVERIFY(pl->current_track_index() == 50);

	IndexSet indexes;
	{ // move indices before cur track
		{
			indexes << 1;
			indexes << 2;
			indexes << 3;
			indexes << 4;
		}

		pl->move_tracks(indexes, 75);
		cur_idx = pl->current_track_index();
		QVERIFY(cur_idx == 46);
	}

	{ // move before, after and with current track
		indexes.clear();
		{
			indexes << 65;		// new 12
			indexes << 32;		// new 10
			indexes << 46;		// new 11
			indexes << 6;		// new 9
		}

		pl->move_tracks(indexes, 10);
		cur_idx = pl->current_track_index();
		QVERIFY(cur_idx == 11);
	}

	{ // move current track
		indexes.clear();
		{
			indexes << 11;		// new 20 - indexes.size() = 18
			indexes << 12;		// new 19
		}

		pl->move_tracks(indexes, 20);
		cur_idx = pl->current_track_index();
		QVERIFY(cur_idx == 18);
	}

	{ // check if uids haven't changed
		QList<UniqueId> pl_uids_after = pl_tracks.unique_ids();
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

		pl->remove_tracks(indexes);
		cur_idx = pl->current_track_index();
		QVERIFY(cur_idx == 14);
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

	pl->remove_tracks(indexes);
	cur_idx = pl->current_track_index();
	QVERIFY(cur_idx == -1);
}

QTEST_GUILESS_MAIN(PlaylistTest)

#include "PlaylistTest.moc"

