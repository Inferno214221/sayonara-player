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
