#include "SayonaraTest.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Library/Filter.h"
// access working directory with Test::Base::tempPath("somefile.txt");

class CissearchTest : 
    public Test::Base
{
    Q_OBJECT

    public:
        CissearchTest() :
            Test::Base("CissearchTest")
        {}

    private slots:
		void genreListTest();
};

void CissearchTest::genreListTest()
{
	MetaData md;

	const QStringList genreNames
	{
		"1Pop",
		"2poP",
		"3Rock",
		"4psy rock",
		"5hip-hop",
		"6Hip Hop"
	};

	for(const QString& name : genreNames)
	{
		md.addGenre(Genre(name));
	}

	Library::SearchModeMask smm;
	QString cis;

	QStringList genreList = md.genresToList();
	genreList.sort();

	smm = 0;
	cis = Library::Utils::convertSearchstring(genreList.join(","), smm);
	QVERIFY(cis == "1Pop,2poP,3Rock,4psy rock,5hip-hop,6Hip Hop");

	smm = Library::SearchMode::CaseInsensitve;
	cis = Library::Utils::convertSearchstring(genreList.join(","), smm);
	QVERIFY(cis == "1pop,2pop,3rock,4psy rock,5hip-hop,6hip hop");

	smm = Library::SearchMode::NoSpecialChars;
	cis = Library::Utils::convertSearchstring(genreList.join(","), smm);
	QVERIFY(cis == "1Pop2poP3Rock4psyrock5hiphop6HipHop");

	smm = Library::SearchMode::CaseInsensitve | Library::SearchMode::NoSpecialChars;
	cis = Library::Utils::convertSearchstring(genreList.join(","), smm);
	QVERIFY(cis == "1pop2pop3rock4psyrock5hiphop6hiphop");
}

QTEST_GUILESS_MAIN(CissearchTest)
#include "CissearchTest.moc"
