#include "test/Common/SayonaraTest.h"

#include "Utils/Ranges.h"

#include <QList>

// access working directory with Test::Base::tempPath("somefile.txt");

class Ranges : 
    public Test::Base
{
    Q_OBJECT

    public:
        Ranges() :
            Test::Base("Ranges")
        {}

    private slots:
        void test();
};

void Ranges::test()
{
	{
		const auto list = QList<int>();
		const auto ranges = Util::getRangesFromList(list);

		QVERIFY(ranges.empty());
	}

	{
		const auto list = QList<int>() << 1;
		const auto ranges = Util::getRangesFromList(list);

		QVERIFY(ranges.size() == 1);
		QVERIFY(ranges[0].first == 0);
		QVERIFY(ranges[0].second == 0);
	}

	{
		const auto list = QList<int>() << 1 << 2 << 3 << 4 << 5;
		const auto ranges = Util::getRangesFromList(list);

		QVERIFY(ranges.size() == 1);
		QVERIFY(ranges[0].first == 0);
		QVERIFY(ranges[0].second == 4);
	}

	{
		const auto list = QList<int>() << 2 << 1 << 5 << 1 << 3 << 3 << 4;
		const auto preparedList = Util::prepareContainerForRangeCalculation(list);

		QVERIFY(preparedList.size() == 5);
		QVERIFY(preparedList[0] == 1);
		QVERIFY(preparedList[1] == 2);
		QVERIFY(preparedList[2] == 3);
		QVERIFY(preparedList[3] == 4);
		QVERIFY(preparedList[4] == 5);

		const auto ranges = Util::getRangesFromList(preparedList);

		QVERIFY(ranges.size() == 1);
		QVERIFY(ranges[0].first == 0);
		QVERIFY(ranges[0].second == 4);
	}

	{
		const auto list = QList<int>() << 1 << 2 << 5 << 8 << 9;
		const auto ranges = Util::getRangesFromList(list);
		QVERIFY(ranges.size() == 3);

		const auto [from0, to0] = ranges[0];
		QVERIFY(from0 == 0);
		QVERIFY(to0 == 1);

		const auto [from1, to1] = ranges[1];
		QVERIFY(from1 == 2);
		QVERIFY(to1 == 2);

		const auto [from2, to2] = ranges[2];
		QVERIFY(from2 == 3);
		QVERIFY(to2 == 4);
	}
}

QTEST_GUILESS_MAIN(Ranges)
#include "Ranges.moc"
