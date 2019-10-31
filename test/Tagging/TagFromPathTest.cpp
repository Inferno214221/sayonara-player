#include "SayonaraTest.h"

#include "Components/Tagging/Expression.h"
#include "Utils/MetaData/MetaDataList.h"

class TagFromPathTest : public SayonaraTest
{
	Q_OBJECT

public:
	TagFromPathTest() :
		SayonaraTest("TagFromPathTest")
	{}

	~TagFromPathTest() override = default;

	private slots:
		void apply_regex_test();
};


void TagFromPathTest::apply_regex_test()
{
	const QStringList tag_strings
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

	for(const QString& tag_str : tag_strings)
	{
		for(const QString& path : paths)
		{
			Tagging::Expression e(tag_str, path);
			QVERIFY(e.is_valid() == true);
		}
	}

	{
		QString path = paths[0];
		MetaData md(path);

		Tagging::Expression e1(tag_strings[0], path);
		e1.apply(md);
		QVERIFY(md.track_number() == 2);
		QVERIFY(md.title() == "The Watcher (feat. Eminem & Knoc-Turn'al)");
	}

	{
		QString path = paths[1];
		MetaData md(path);

		Tagging::Expression e1(tag_strings[1], path);
		e1.apply(md);
		QVERIFY(md.track_number() == 4);
		QVERIFY(md.title() == "Still D.R.E. (feat. Snoop Dogg)");
		QVERIFY(md.artist() == "Dr. Dre");
	}

	{
		QString path = paths[2];
		MetaData md(path);

		Tagging::Expression e1(tag_strings[2], path);
		e1.apply(md);
		QVERIFY(md.track_number() == 6);
		QVERIFY(md.title() == "Xxplosive (feat. Hittman, Kurupt, Nate Dogg & Six-Two)");
		QVERIFY(md.artist() == "Dr. Dre");
		QVERIFY(md.year() == 1999);
	}

	{
		QString path = paths[3];
		MetaData md(path);

		Tagging::Expression e1(tag_strings[3], path);
		e1.apply(md);
		QVERIFY(md.track_number() == 7);
		QVERIFY(md.title() == "What's The Difference (feat. Eminem & Xzibit)");
		QVERIFY(md.artist() == "Dr. Dre");
		QVERIFY(md.year() == 1999);
		QVERIFY(md.album() == "2001");
	}
}

QTEST_GUILESS_MAIN(TagFromPathTest)

#include "TagFromPathTest.moc"
