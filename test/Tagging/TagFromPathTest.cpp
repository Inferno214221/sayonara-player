#include <QTest>
#include <QObject>
#include "Components/Tagging/Expression.h"

class TagFromPathTest : public QObject
{
	Q_OBJECT

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
		"/media/Sound/Dr. Dre/1999 2001/06. Xxplosive (feat. Hittman, Kurupt, Nate Dogg & Six-Two).m4a"
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
}

QTEST_GUILESS_MAIN(TagFromPathTest)

#include "TagFromPathTest.moc"
