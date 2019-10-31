#include "SayonaraTest.h"
#include "Utils/Set.h"
#include <algorithm>

class SetTest : public QObject
{
	Q_OBJECT

private slots:
	void test_equality();
	void test_duplicates();
};


void SetTest::test_equality()
{
	QList<int> indexes
	{
		4, 5, 8, 1, 0, 2, 3, 6
	};

	Util::Set<int> index_set;
	for(int i : indexes)
	{
		index_set << i;
	}

	{
		QList<int> set_list = index_set.toList();

		QVERIFY(index_set.count() == indexes.count());
		QVERIFY(set_list != indexes);

		std::sort(set_list.begin(), set_list.end());
		std::sort(indexes.begin(), indexes.end());
		QVERIFY(set_list == indexes);
	}

	{ // assure that set is sorted when running via iterator
		QList<int> set_list = index_set.toList();
		std::sort(set_list.begin(), set_list.end());

		QList<int> set_list2;
		for(auto it=index_set.begin(); it != index_set.end(); it++)
		{
			set_list2 << *it;
		}

		QVERIFY(set_list2 == set_list);
	}
}

void SetTest::test_duplicates()
{
	// 3 and 4 are doubled
	QList<int> indexes
	{
		4, 5, 8, 1, 0, 2, 3, 6, 3, 4
	};

	Util::Set<int> index_set;
	for(int i : indexes)
	{
		index_set << i;
	}

	QVERIFY(index_set.count() == indexes.count() - 2);
}

QTEST_GUILESS_MAIN(SetTest)

#include "SetTest.moc"

