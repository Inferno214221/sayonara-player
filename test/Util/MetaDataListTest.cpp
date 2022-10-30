#include "test/Common/SayonaraTest.h"

#include "Utils/FileUtils.h"
#include "Utils/RandomGenerator.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <algorithm>

class MetaDataListTest :
	public Test::Base
{
	Q_OBJECT

	public:
		MetaDataListTest() :
			Test::Base("MetaDataListTest") {}

		~MetaDataListTest() override = default;

	private slots:
		void insert_test();
		void remove_test();
		void move_test();
		void append_unique_test();
};

static MetaDataList create_v_md(int min, int max)
{
	MetaDataList v_md;
	for(int i = min; i < max; i++)
	{
		MetaData md;
		md.setId(i);

		v_md << md;
	}

	return v_md;
}

void MetaDataListTest::insert_test()
{
	MetaDataList v_md_orig = create_v_md(0, 53);
	MetaDataList inserted_md = create_v_md(100, 105);
	MetaDataList v_md = v_md_orig;

	QVERIFY(v_md.size() == v_md_orig.size());

	int insert_idx = 8;

	QList<UniqueId> unique_ids = Util::uniqueIds(inserted_md);
	QList<UniqueId> unique_ids2;

	v_md.insertTracks(inserted_md, insert_idx);
	QVERIFY(v_md.size() == v_md_orig.size() + inserted_md.size());

	QList<int> expected_ids;
	for(int i = 0; i < insert_idx; i++)
	{
		expected_ids << v_md[i].id();
	}

	for(int i = 0; i < inserted_md.count(); i++)
	{
		expected_ids << inserted_md[i].id();
		unique_ids2 << v_md[i + insert_idx].uniqueId();
	}

	QVERIFY(unique_ids != unique_ids2);

	for(int i = insert_idx; i < v_md_orig.count(); i++)
	{
		expected_ids << v_md_orig[i].id();
	}

	QVERIFY(v_md.count() == expected_ids.size());

	for(int i = 0; i < v_md.count(); i++)
	{
		QVERIFY(v_md[i].id() == expected_ids[i]);
	}

	{ // some invalid index tests
		MetaDataList v_md_invalid;
		MetaData md;
		v_md_invalid.insertTrack(md, 4);
		QVERIFY(v_md_invalid.size() == 1);

		v_md_invalid.clear();
		v_md_invalid.insertTrack(md, -1);
		QVERIFY(v_md_invalid.size() == 1);
	}
}

void MetaDataListTest::remove_test()
{
	MetaDataList v_md = create_v_md(0, 100);
	auto old_size = v_md.size();

	int remove_start = 15;
	int remove_end = 30;

	IndexSet remove_indexes;
	for(int i = remove_start; i < remove_end; i++)
	{
		remove_indexes << i;
	}

	v_md.removeTracks(remove_indexes);

	QVERIFY(v_md.size() == (old_size - remove_indexes.size()));

	for(int i = 0; i < v_md.count(); i++)
	{
		if(i < remove_start)
		{
			QVERIFY(v_md[i].id() == i);
		}

		else
		{
			QVERIFY(v_md[i].id() == (i + remove_indexes.count()));
		}
	}

	{ // some invalid index tests
		MetaDataList v_md_invalid;
		v_md_invalid.removeTrack(3);
		QVERIFY(v_md_invalid.size() == 0);

		// insert two tracks
		MetaDataList md_insert;
		md_insert << MetaData() << MetaData();
		v_md_invalid << md_insert;

		// remove tracks with invalid indexes
		QVERIFY(v_md_invalid.size() == 2);
		v_md_invalid.removeTrack(-1);
		v_md_invalid.removeTrack(2);

		// remove track with real index
		QVERIFY(v_md_invalid.size() == 2);
		v_md_invalid.removeTrack(0);
		QVERIFY(v_md_invalid.size() == 1);

		MetaData md;
		md.setFilepath("Somestuff.mp3");
		v_md_invalid << md;
		QVERIFY(v_md_invalid.size() == 2);

		IndexSet idxs;
		{
			idxs << 2 << -1 << 4;
		}

		v_md_invalid.removeTracks(idxs);
		QVERIFY(v_md_invalid.size() == 2);

		idxs.clear();
		{
			idxs << 0 << -1 << 4;
		}

		v_md_invalid.removeTracks(idxs);
		QVERIFY(v_md_invalid.size() == 1);
	}
}

void MetaDataListTest::move_test()
{
	const MetaDataList v_md_orig = create_v_md(0, 10);
	MetaDataList v_md = v_md_orig;
	QList<UniqueId> unique_ids = Util::uniqueIds(v_md);
	QList<UniqueId> unique_ids2;

	IndexSet move_indexes;

	{ // move some items

		// O, O, O, X, X, X, O, O, O, O
		// O, X, X, X, O, O, O, O, O, O
		{
			move_indexes << 3 << 4 << 5;
		};

		v_md.moveTracks(move_indexes, 1);
		unique_ids2 = Util::uniqueIds(v_md);
		QVERIFY(unique_ids != unique_ids2);

		std::sort(unique_ids.begin(), unique_ids.end());
		std::sort(unique_ids2.begin(), unique_ids2.end());

		QVERIFY(unique_ids == unique_ids2);

		QList<int> expected_ids
			{
				0, 3, 4, 5, 1, 2, 6, 7, 8, 9
			};

		QVERIFY(expected_ids.count() == v_md.count());
		for(int i = 0; i < expected_ids.count(); i++)
		{
			QVERIFY(expected_ids[i] == v_md[i].id());
		}
	}

	{ // and back again
		// O, X, X, X, O, O, O, O, O, O
		// O, O, O, X, X, X, O, O, O, O
		move_indexes.clear();
		{
			move_indexes << 1 << 2 << 3;
		};

		v_md.moveTracks(move_indexes, 6);

		unique_ids2 = Util::uniqueIds(v_md);
		QVERIFY(unique_ids == unique_ids2);

		for(int i = 0; i < v_md.count(); i++)
		{
			QVERIFY(i == v_md[i].id());
		}
	}

	{ // move behind last index
		v_md = v_md_orig;
		unique_ids = Util::uniqueIds(v_md);
		move_indexes.clear();
		{
			move_indexes << 1 << 2 << 3;
		};

		v_md.moveTracks(move_indexes, 11);

		unique_ids2 = Util::uniqueIds(v_md);

		std::sort(unique_ids.begin(), unique_ids.end());
		std::sort(unique_ids2.begin(), unique_ids2.end());
		QVERIFY(unique_ids == unique_ids2);

		QList<int> expected_ids
			{
				0, 4, 5, 6, 7, 8, 9, 1, 2, 3
			};

		for(int i = 0; i < expected_ids.count(); i++)
		{
			QVERIFY(expected_ids[i] == v_md[i].id());
		}
	}

	{ // move before first index
		v_md = v_md_orig;
		move_indexes.clear();
		{
			move_indexes << 4 << 7 << 8;
		};

		v_md.moveTracks(move_indexes, -4);
		QList<int> expected_ids
			{
				4, 7, 8, 0, 1, 2, 3, 5, 6, 9
			};

		for(int i = 0; i < expected_ids.count(); i++)
		{
			QVERIFY(expected_ids[i] == v_md[i].id());
		}
	}
}

void MetaDataListTest::remove_duplicate_test()
{
	MetaDataList v_md_orig = create_v_md(0, 100);

	{ // % 10
		MetaDataList v_md = v_md_orig;
		for(int i=0; i<v_md.count(); i++)
		{
			QString p = QString("/some/path/%1.mp3").arg(i % 10);
			v_md[i].setFilepath(p);
		}

		v_md.removeDuplicates();
		QVERIFY(v_md.size() == 10);

		for(int i=0; i<v_md.count(); i++)
		{
			int id = v_md[i].id();
			QVERIFY(id == i);
		}
	}
	{ // / 3
		MetaDataList v_md = v_md_orig;
		for(int i=0; i<v_md.count(); i++)
		{
			QString p = QString("/some/path/%1.mp3").arg(i / 3);
			v_md[i].setFilepath(p);
		}

		v_md.removeDuplicates();
		QVERIFY(v_md.size() == (v_md_orig.size() + 2) / 3);

		for(int i=0; i<v_md.count(); i++)
		{
			int id = v_md[i].id();
			QVERIFY(id == i * 3);
		}
	}

	{ // % 99
		MetaDataList v_md = v_md_orig;
		for(int i=0; i<v_md.count(); i++)
		{
			QString p = QString("/some/path/%1.mp3").arg(i % 99);
			v_md[i].setFilepath(p);
		}

		v_md.removeDuplicates();
		QVERIFY(v_md.size() == 99);

		for(int i=0; i<v_md.count(); i++)
		{
			int id = v_md[i].id();
			QVERIFY(id == i);
		}
	}


	{ // all the same
		MetaDataList v_md = v_md_orig;
		for(int i=0; i<v_md.count(); i++)
		{
			QString p = QString("/some/path/hallo.mp3");
			v_md[i].setFilepath(p);
		}

		v_md.removeDuplicates();
		QVERIFY(v_md.size() == 1);
	}

	{
		MetaDataList v_md = v_md_orig;
		for(int i=0; i<v_md.count(); i++)
		{
			QString p = QString("/some/path/%1.mp3").arg(i);
			v_md[i].setFilepath(p);
		}

		v_md.removeDuplicates();
		QVERIFY(v_md.size() == v_md_orig.size());
	}
}

void MetaDataListTest::append_unique_test()
{
	MetaDataList v_md_orig = create_v_md(0, 100);
	MetaDataList v_md = v_md_orig;
	MetaDataList v_md2 = create_v_md(70, 170);
	for(MetaData& md: v_md)
	{
		QString p = QString("/some/path/%1.mp3").arg(md.id());
		md.setFilepath(p);
	}

	for(MetaData& md: v_md2)
	{
		QString p = QString("/some/path/%1.mp3").arg(md.id());
		md.setFilepath(p);
	}

	v_md.appendUnique(v_md2);

	QVERIFY(v_md.size() == v_md_orig.size() + 70);
	for(int i = 0; i < v_md.count(); i++)
	{
		QVERIFY(v_md[i].id() == i);
	}
}

QTEST_GUILESS_MAIN(MetaDataListTest)

#include "MetaDataListTest.moc"
