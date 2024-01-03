/* PlaylistModeTest.cpp
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

#include "Utils/Playlist/PlaylistMode.h"

// access working directory with Test::Base::tempPath("somefile.txt");

class PlaylistModeTest : 
    public Test::Base
{
    Q_OBJECT

    public:
        PlaylistModeTest() :
            Test::Base("PlaylistModeTest")
        {}

    private slots:
        void test();
};

void PlaylistModeTest::test()
{
	using Playlist::Mode;
	auto mode1 = Mode();
	auto mode2 = Mode();

	QVERIFY(mode1 == mode2);

	mode2.setAppend(!Mode::isActive(mode2.append()));
	QVERIFY(!(mode1 == mode2));
	QVERIFY(mode1.append() != mode2.append());

	mode2.setDynamic(!Mode::isActive(mode2.dynamic()));
	QVERIFY(!(mode1 == mode2));
	QVERIFY(mode1.dynamic() != mode2.dynamic());

	mode2.setGapless(!Mode::isActive(mode2.gapless()));
	QVERIFY(!(mode1 == mode2));
	QVERIFY(mode1.gapless() != mode2.gapless());

	mode2.setRep1(!Mode::isActive(mode2.rep1()));
	QVERIFY(!(mode1 == mode2));
	QVERIFY(mode1.rep1() != mode2.rep1());

	mode2.setShuffle(!Mode::isActive(mode2.shuffle()));
	QVERIFY(!(mode1 == mode2));
	QVERIFY(mode1.shuffle() != mode2.shuffle());

	mode1 = mode2;
	QVERIFY(mode1 == mode2);
}

QTEST_GUILESS_MAIN(PlaylistModeTest)
#include "PlaylistModeTest.moc"
