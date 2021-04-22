#include "SayonaraTest.h"

#include "Components/Tagging/Expression.h"
#include "Utils/MetaData/MetaDataList.h"

class TagFromPathTest :
	public Test::Base
{
	Q_OBJECT

	public:
		TagFromPathTest() :
			Test::Base("TagFromPathTest") {}

		~TagFromPathTest() override = default;

	private slots:
		void applyRegexTest();
		void unknownTagTest();
		void noMatchingStringTest();
		void wrongTrackNumTest();
		void emptyTagResultTest();
		void badNumericTest();
		void emptyTagStringTest();
};

void TagFromPathTest::applyRegexTest()
{
	const QStringList tagStrings
		{
			"/media/Sound/Dr. Dre/1999 2001/<tracknum>. <title>.m4a",
			"/media/Sound/<artist>/1999 2001/<tracknum>. <title>.m4a",
			"/media/Sound/<artist>/<year> 2001/<tracknum>. <title>.m4a",
			"/media/Sound/<artist>/<year> <album>/<tracknum>. <title>.m4a"
		};

	const QStringList paths
		{
			"/media/Sound/Dr. Dre/1999 2001/02. The Watcher (feat. Eminem & Knoc-Turn'al).m4a",
			"/media/Sound/Dr. Dre/1999 2001/04. Still D.R.E. (feat. Snoop Dogg).m4a",
			"/media/Sound/Dr. Dre/1999 2001/06. Xxplosive (feat. Hittman, Kurupt, Nate Dogg & Six-Two).m4a",
			"/media/Sound/Dr. Dre/1999 2001/07. What's The Difference (feat. Eminem & Xzibit).m4a"
		};

	for(const auto& tagString : tagStrings)
	{
		for(const auto& path : paths)
		{
			const Tagging::Expression expression(tagString, path);
			QVERIFY(expression.isValid());
		}
	}

	{
		const auto path = paths[0];
		auto track = MetaData(path);

		Tagging::Expression expression(tagStrings[0], path);
		expression.apply(track);

		QVERIFY(track.trackNumber() == 2);
		QVERIFY(track.title() == "The Watcher (feat. Eminem & Knoc-Turn'al)");
	}

	{
		const auto path = paths[1];
		auto track = MetaData(path);

		Tagging::Expression expression(tagStrings[1], path);
		expression.apply(track);

		QVERIFY(track.trackNumber() == 4);
		QVERIFY(track.title() == "Still D.R.E. (feat. Snoop Dogg)");
		QVERIFY(track.artist() == "Dr. Dre");
	}

	{
		const auto path = paths[2];
		auto track = MetaData(path);

		Tagging::Expression expression(tagStrings[2], path);
		expression.apply(track);

		QVERIFY(track.trackNumber() == 6);
		QVERIFY(track.title() == "Xxplosive (feat. Hittman, Kurupt, Nate Dogg & Six-Two)");
		QVERIFY(track.artist() == "Dr. Dre");
		QVERIFY(track.year() == 1999);
	}

	{
		const auto path = paths[3];
		auto track = MetaData(path);

		Tagging::Expression expression(tagStrings[3], path);
		expression.apply(track);

		QVERIFY(track.trackNumber() == 7);
		QVERIFY(track.title() == "What's The Difference (feat. Eminem & Xzibit)");
		QVERIFY(track.artist() == "Dr. Dre");
		QVERIFY(track.year() == 1999);
		QVERIFY(track.album() == "2001");
	}
}

void TagFromPathTest::unknownTagTest()
{
	const auto tagString = "/media/Sound/Dr. Dre/<strange tag>1999 2001/<tracknum>. <title>.m4a";
	const auto path = "/media/Sound/Dr. Dre/1999 2001/02. The Watcher (feat. Eminem & Knoc-Turn'al).m4a";
	const auto expression = Tagging::Expression(tagString, path);

	QVERIFY(!expression.isValid());
	QVERIFY(expression.capturedTags().isEmpty());
}

void TagFromPathTest::noMatchingStringTest()
{
	const auto tagString = "/media/Sound/1999 2001/<tracknum>. <title>.m4a";
	const auto path = "/media/Sound/Dr. Dre/1999 2001/02. The Watcher (feat. Eminem & Knoc-Turn'al).m4a";
	const auto expression = Tagging::Expression(tagString, path);

	QVERIFY(!expression.isValid());
	QVERIFY(expression.capturedTags().isEmpty());
}

void TagFromPathTest::wrongTrackNumTest()
{
	const auto tagString = "/media/Sound/Dr. Dre/1999 2001/03. <title>.m4a";
	const auto path = "/media/Sound/Dr. Dre/1999 2001/02. The Watcher (feat. Eminem & Knoc-Turn'al).m4a";
	const auto expression = Tagging::Expression(tagString, path);

	QVERIFY(!expression.isValid());
	QVERIFY(expression.capturedTags().isEmpty());
}

void TagFromPathTest::emptyTagResultTest()
{
	const auto tagString = "/media/Sound/Dr. Dre/1999 2001/02. <title>.m4a<album>";
	const auto path = "/media/Sound/Dr. Dre/1999 2001/02. The Watcher (feat. Eminem & Knoc-Turn'al).m4a";
	const auto expression = Tagging::Expression(tagString, path);

	QVERIFY(!expression.isValid());
	QVERIFY(expression.capturedTags().isEmpty());
}

void TagFromPathTest::badNumericTest()
{
	const auto tagString = "/media/Sound/Dr. Dre/<year>2001/02. <title>.m4a";
	const auto path = "/media/Sound/Dr. Dre/1999 2001/02. The Watcher (feat. Eminem & Knoc-Turn'al).m4a";
	const auto expression = Tagging::Expression(tagString, path);

	QVERIFY(!expression.isValid());
	QVERIFY(expression.capturedTags().isEmpty());
}

void TagFromPathTest::emptyTagStringTest()
{
	const auto tagString = "";
	const auto path = "/media/Sound/Dr. Dre/1999 2001/02. The Watcher (feat. Eminem & Knoc-Turn'al).m4a";
	const auto expression = Tagging::Expression(tagString, path);

	QVERIFY(!expression.isValid());
	QVERIFY(expression.capturedTags().isEmpty());
}

QTEST_GUILESS_MAIN(TagFromPathTest)

#include "TagFromPathTest.moc"
