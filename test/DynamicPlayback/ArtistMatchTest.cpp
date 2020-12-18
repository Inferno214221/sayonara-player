#include "SayonaraTest.h"
#include "Components/DynamicPlayback/ArtistMatch.h"
// access working directory with Test::Base::tempPath("somefile.txt");

using DynamicPlayback::ArtistMatch;

namespace
{
	ArtistMatch createMatch()
	{
		ArtistMatch match;
		match.add(ArtistMatch::Entry("Artist1", "MBID1", 0));
		match.add(ArtistMatch::Entry("Artist2", "MBID2", 0.9342445));
		match.add(ArtistMatch::Entry("Artist3", "MBID3", 0.27234523));
		match.add(ArtistMatch::Entry("Artist4", "MBID4", 0.1345));
		match.add(ArtistMatch::Entry("Artist5", "MBID5", 0.675345));

		return match;
	}
}

class ArtistMatchTest :
    public Test::Base
{
    Q_OBJECT

    public:
		ArtistMatchTest() :
            Test::Base("ArtistMatch")
        {}

    private slots:
        void testCopy();
        void testConvert();
};

void ArtistMatchTest::testCopy()
{
	const auto match = createMatch();

	QVERIFY(match.isValid());
	QVERIFY(match.get(ArtistMatch::Quality::Poor).size() == 3);
	QVERIFY(match.get(ArtistMatch::Quality::Good).size() == 1);
	QVERIFY(match.get(ArtistMatch::Quality::VeryGood).size() == 1);
	QVERIFY(match.get(ArtistMatch::Quality::Excellent).size() == 2);
}

void ArtistMatchTest::testConvert()
{
	const auto match = createMatch();

	const auto string = match.toString();
	const auto match2 = ArtistMatch::fromString(string);

	QVERIFY(match == match2);
}

QTEST_GUILESS_MAIN(ArtistMatchTest)
#include "ArtistMatchTest.moc"
