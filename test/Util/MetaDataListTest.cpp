#include <QObject>
#include <QTest>
#include <QDebug>

#include "Utils/FileUtils.h"
#include "Utils/RandomGenerator.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <algorithm>

class MetaDataListTest : public QObject
{
	Q_OBJECT

private slots:
	void insert_test();
	void remove_test();
	void move_test();
	void remove_duplicate_test();
	void append_unique_test();
};

static MetaDataList create_v_md(int min, int max)
{
	MetaDataList v_md;
	for(int i=min; i<max; i++)
	{
		MetaData md;
		md.set_id(i);

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

	QList<UniqueId> unique_ids = inserted_md.unique_ids();
	QList<UniqueId> unique_ids2;

	v_md.insert_tracks(inserted_md, insert_idx);
	QVERIFY(v_md.size() == v_md_orig.size() + inserted_md.size());

	QList<int> expected_ids;
	for(int i=0; i<insert_idx; i++) {
		expected_ids << v_md[i].id();
	}

	for(int i=0; i<inserted_md.count(); i++) {
		expected_ids << inserted_md[i].id();
		unique_ids2 << v_md[i + insert_idx].unique_id();
	}

	QVERIFY(unique_ids != unique_ids2);

	for(int i=insert_idx; i<v_md_orig.count(); i++)
	{
		expected_ids << v_md_orig[i].id();
	}

	QVERIFY(v_md.count() == expected_ids.size());

	for(int i=0; i<v_md.count(); i++)
	{
		QVERIFY(v_md[i].id() == expected_ids[i]);
	}
}


void MetaDataListTest::remove_test()
{
	MetaDataList v_md = create_v_md(0, 100);
	auto old_size = v_md.size();

	int remove_start = 15;
	int remove_end = 30;

	IndexSet remove_indexes;
	for(int i=remove_start; i<remove_end; i++){
		remove_indexes << i;
	}

	v_md.remove_tracks(remove_indexes);

	QVERIFY(v_md.size() == (old_size - remove_indexes.size()));

	for(int i=0; i<v_md.count(); i++)
	{
		if(i<remove_start)
		{
			QVERIFY(v_md[i].id() == i);
		}

		else {
			QVERIFY(v_md[i].id() == (i + remove_indexes.count()));
		}
	}
}

void MetaDataListTest::move_test()
{
	MetaDataList v_md_orig = create_v_md(0, 10);
	MetaDataList v_md = v_md_orig;
	QList<UniqueId> unique_ids = v_md.unique_ids();
	QList<UniqueId> unique_ids2;

	IndexSet move_indexes;

	{ // move some items

		// O, O, O, X, X, X, O, O, O, O
		// O, X, X, X, O, O, O, O, O, O
		{
			move_indexes << 3;
			move_indexes << 4;
			move_indexes << 5;
		};

		v_md.move_tracks(move_indexes, 1);
		unique_ids2 = v_md.unique_ids();
		QVERIFY(unique_ids != unique_ids2);

		std::sort(unique_ids.begin(), unique_ids.end());
		std::sort(unique_ids2.begin(), unique_ids2.end());

		QVERIFY(unique_ids == unique_ids2);

		QList<int> expected_ids
		{
			0, 3, 4, 5, 1, 2, 6, 7, 8, 9
		};

		QVERIFY(expected_ids.count() == v_md.count());
		for(int i=0; i<expected_ids.count(); i++)
		{
			QVERIFY(expected_ids[i] == v_md[i].id());
		}
	}

	{ // and back again
		// O, X, X, X, O, O, O, O, O, O
		// O, O, O, X, X, X, O, O, O, O
		move_indexes.clear();
		{
			move_indexes << 1;
			move_indexes << 2;
			move_indexes << 3;
		};

		v_md.move_tracks(move_indexes, 6);

		unique_ids2 = v_md.unique_ids();
		QVERIFY(unique_ids == unique_ids2);

		for(int i=0; i<v_md.count(); i++)
		{
			QVERIFY(i == v_md[i].id());
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
			v_md[i].set_filepath(p);
		}

		v_md.remove_duplicates();
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
			v_md[i].set_filepath(p);
		}

		v_md.remove_duplicates();
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
			v_md[i].set_filepath(p);
		}

		v_md.remove_duplicates();
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
			v_md[i].set_filepath(p);
		}

		v_md.remove_duplicates();
		QVERIFY(v_md.size() == 1);
	}
}

void MetaDataListTest::append_unique_test()
{
	MetaDataList v_md_orig = create_v_md(0, 100);
	MetaDataList v_md = v_md_orig;
	MetaDataList v_md2 = create_v_md(70, 170);
	for(MetaData& md : v_md)
	{
		QString p = QString("/some/path/%1.mp3").arg(md.id());
		md.set_filepath(p);
	}

	for(MetaData& md : v_md2)
	{
		QString p = QString("/some/path/%1.mp3").arg(md.id());
		md.set_filepath(p);
	}

	v_md.append_unique(v_md2);

	QVERIFY(v_md.size() == v_md_orig.size() + 70);
	for(int i=0; i<v_md.count(); i++)
	{
		QVERIFY(v_md[i].id() == i);
	}
}

QTEST_GUILESS_MAIN(MetaDataListTest)

#include "MetaDataListTest.moc"
