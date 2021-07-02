#include "SayonaraTest.h"
// access working directory with Test::Base::tempPath("somefile.txt");

#include "Utils/Library/Filter.h"

using Library::Filter;

class LibraryFilterTest : 
    public Test::Base
{
    Q_OBJECT

    public:
        LibraryFilterTest() :
            Test::Base("LibraryFilterTest")
        {}

    private slots:
        void testConstructCopyCompare();
        void testFilterText();
        void testMode();
		void testClear();
};

void LibraryFilterTest::testConstructCopyCompare()
{
	auto searchModeMask = (Library::SearchMode::CaseInsensitve | Library::SearchMode::NoSpecialChars);
	auto filter = Library::Filter();
	{ // test standard constructor
		QVERIFY(filter.mode() == Library::Filter::Fulltext);
		QVERIFY(filter.cleared());
		QVERIFY(filter.searchModeFiltertext(false).isEmpty());
		QVERIFY(filter.isUseable() == false);
		QVERIFY(filter.count() == 0);
	}

	filter.setFiltertext("search1,search2", searchModeMask);
	filter.setMode(Filter::Mode::Genre);

	auto filter2 = filter;
	{ // compare filters
		QVERIFY(filter2 == filter);
		QVERIFY(filter2.mode() == filter.mode());
		QVERIFY(filter2.cleared() == filter.cleared());
		QVERIFY(filter2.searchModeFiltertext(false) == filter.searchModeFiltertext(false));
		QVERIFY(filter2.filtertext(false) == filter.searchModeFiltertext(false));
		QVERIFY(filter2.searchModeFiltertext(true) == filter.searchModeFiltertext(true));
		QVERIFY(filter2.filtertext(true) == filter.searchModeFiltertext(true));
		QVERIFY(filter2.count() == filter.count());
	}
	{ // compare cleared filters
		filter.clear();
		filter2.clear();

		QVERIFY(filter2 == filter);
		QVERIFY(filter2.mode() == filter.mode());
		QVERIFY(filter2.cleared() == filter.cleared());
		QVERIFY(filter2.searchModeFiltertext(false) == filter.searchModeFiltertext(false));
		QVERIFY(filter2.filtertext(false) == filter.searchModeFiltertext(false));
		QVERIFY(filter2.searchModeFiltertext(true) == filter.searchModeFiltertext(true));
		QVERIFY(filter2.filtertext(true) == filter.searchModeFiltertext(true));
		QVERIFY(filter2.count() == filter.count());
	}
}

void LibraryFilterTest::testFilterText()
{
	const auto searchModeMask = (Library::SearchMode::CaseInsensitve | Library::SearchMode::NoSpecialChars);

	auto filter = Filter();
	filter.setFiltertext("searchBla1,search$BLupp2", searchModeMask);
	QVERIFY(filter.count() == 2);

	const auto searchModeFiltertext = filter.searchModeFiltertext(false);
	QVERIFY(searchModeFiltertext.count() == 2);
	QVERIFY(searchModeFiltertext[0] == "searchbla1");
	QVERIFY(searchModeFiltertext[1] == "searchblupp2");

	const auto searchModeFiltertextPercent = filter.searchModeFiltertext(true);
	QVERIFY(searchModeFiltertextPercent.count() == 2);
	QVERIFY(searchModeFiltertextPercent[0] == "%searchbla1%");
	QVERIFY(searchModeFiltertextPercent[1] == "%searchblupp2%");

	const auto filtertext = filter.filtertext(false);
	QVERIFY(filtertext.count() == 2);
	QVERIFY(filtertext[0] == "searchBla1");
	QVERIFY(filtertext[1] == "search$BLupp2");

	const auto filtertextPercent = filter.filtertext(true);
	QVERIFY(filtertextPercent.count() == 2);
	QVERIFY(filtertextPercent[0] == "%searchBla1%");
	QVERIFY(filtertextPercent[1] == "%search$BLupp2%");
}

void LibraryFilterTest::testMode()
{
	const auto searchModeMask = (Library::SearchMode::CaseInsensitve | Library::SearchMode::NoSpecialChars);
	{ // test fulltext
		auto filter = Filter();
		filter.setFiltertext("searchBla1,search$BLupp2", searchModeMask);
		filter.setMode(Filter::Mode::Fulltext);
		QVERIFY(filter.isUseable());
	}
	{ // test Invalid
		auto filter2 = Filter();
		filter2.setFiltertext("searchBla1,search$BLupp2", searchModeMask);
		filter2.setMode(Filter::Mode::Invalid);
		QVERIFY(filter2.isUseable() == false);
	}
	{ // test InvalidGenre
		auto filter3 = Filter();
		filter3.setFiltertext("searchBla1,search$BLupp2", searchModeMask);
		filter3.setMode(Filter::Mode::InvalidGenre);
		QVERIFY(filter3.isUseable());
	}
	{ // test Filename
		auto filter4 = Filter();
		filter4.setFiltertext("searchBla1,search$BLupp2", searchModeMask);
		filter4.setMode(Filter::Mode::Filename);
		QVERIFY(filter4.isUseable());
	}
	{ // test Genre
		auto filter5 = Filter();
		filter5.setFiltertext("searchBla1,search$BLupp2", searchModeMask);
		filter5.setMode(Filter::Mode::Genre);
		QVERIFY(filter5.isUseable());
	}
}

void LibraryFilterTest::testClear()
{
	const auto searchModeMask = (Library::SearchMode::CaseInsensitve | Library::SearchMode::NoSpecialChars);
	{ // clear filter
		auto filter = Filter();
		filter.setFiltertext("searchBla1,search$BLupp2", searchModeMask);

		QVERIFY(filter.cleared() == false);
		QVERIFY(filter.isUseable());

		filter.clear();
		QVERIFY(filter.cleared());
		QVERIFY(!filter.isUseable());
	}
	{ // empty text means cleared
		auto filter2 = Filter();
		filter2.setFiltertext("", searchModeMask);
		QVERIFY(filter2.cleared());
	}
	{ // empty
		auto filter3 = Filter();
		filter3.setFiltertext("", searchModeMask);
		filter3.setMode(Filter::Mode::InvalidGenre);
		QVERIFY(filter3.cleared() == false);
	}
}

QTEST_GUILESS_MAIN(LibraryFilterTest)
#include "LibraryFilterTest.moc"
