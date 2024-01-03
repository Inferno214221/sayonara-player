/* Ranges.cpp
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
