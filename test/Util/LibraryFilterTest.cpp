#include "test/Common/SayonaraTest.h"

#include "Utils/globals.h"
#include "Utils/Library/Filter.h"
#include "Utils/Settings/Settings.h"

// access working directory with Test::Base::tempPath("somefile.txt");

using Library::Filter;

class LibraryFilterTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LibraryFilterTest() :
			Test::Base("LibraryFilterTest") {}

	private slots:
		[[maybe_unused]] void testConstructCopyCompare();
		[[maybe_unused]] void testFilterText();
		[[maybe_unused]] void testClear();
		[[maybe_unused]] void testFilterLength();
};

// NOLINTNEXTLINE(readability-function-cognitive-complexity,readability-convert-member-functions-to-static)
[[maybe_unused]] void LibraryFilterTest::testConstructCopyCompare()
{
	constexpr const auto MinimumSearchStringLength = 3;
	auto searchModeMask = (+Library::SearchMode::CaseInsensitve | +Library::SearchMode::NoSpecialChars);
	auto filter = Library::Filter();
	{ // test standard constructor
		QVERIFY(filter.mode() == Library::Filter::Fulltext);
		QVERIFY(filter.cleared());
		QVERIFY(filter.searchModeFiltertext(false, searchModeMask).isEmpty());
		QVERIFY(filter.count() == 0);
	}

	filter.setFiltertext("search1,search2");
	filter.setMode(Filter::Mode::Genre);

	auto filter2 = filter;
	{ // compare filters
		QVERIFY(filter2.isEqual(filter, MinimumSearchStringLength));
		QVERIFY(filter2.mode() == filter.mode());
		QVERIFY(filter2.cleared() == filter.cleared());
		QVERIFY(
			filter2.searchModeFiltertext(false, searchModeMask) == filter.searchModeFiltertext(false, searchModeMask));
		QVERIFY(filter2.filtertext(false) == filter.searchModeFiltertext(false, searchModeMask));
		QVERIFY(
			filter2.searchModeFiltertext(true, searchModeMask) == filter.searchModeFiltertext(true, searchModeMask));
		QVERIFY(filter2.filtertext(true) == filter.searchModeFiltertext(true, searchModeMask));
		QVERIFY(filter2.count() == filter.count());
	}
	{ // compare cleared filters
		filter.clear();
		filter2.clear();

		QVERIFY(filter2.isEqual(filter, MinimumSearchStringLength));
		QVERIFY(filter2.mode() == filter.mode());
		QVERIFY(filter2.cleared() == filter.cleared());
		QVERIFY(
			filter2.searchModeFiltertext(false, searchModeMask) == filter.searchModeFiltertext(false, searchModeMask));
		QVERIFY(filter2.filtertext(false) == filter.searchModeFiltertext(false, searchModeMask));
		QVERIFY(
			filter2.searchModeFiltertext(true, searchModeMask) == filter.searchModeFiltertext(true, searchModeMask));
		QVERIFY(filter2.filtertext(true) == filter.searchModeFiltertext(true, searchModeMask));
		QVERIFY(filter2.count() == filter.count());
	}
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity,readability-convert-member-functions-to-static)
[[maybe_unused]] void LibraryFilterTest::testFilterText()
{
	constexpr const auto searchModeMask = (+Library::SearchMode::CaseInsensitve | +Library::SearchMode::NoSpecialChars);

	auto filter = Filter();
	filter.setFiltertext("searchBla1,search$BLupp2");
	QVERIFY(filter.count() == 2);

	const auto searchModeFiltertext = filter.searchModeFiltertext(false, searchModeMask);
	QVERIFY(searchModeFiltertext.count() == 2);
	QVERIFY(searchModeFiltertext[0] == "searchbla1");
	QVERIFY(searchModeFiltertext[1] == "searchblupp2");

	const auto searchModeFiltertextPercent = filter.searchModeFiltertext(true, searchModeMask);
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

[[maybe_unused]] void LibraryFilterTest::testClear() // NOLINT(readability-convert-member-functions-to-static)
{
	{ // clear filter
		auto filter = Filter();
		filter.setFiltertext("searchBla1,search$BLupp2");

		QVERIFY(filter.cleared() == false);

		filter.clear();
		QVERIFY(filter.cleared());
	}
	{ // empty text means cleared
		auto filter2 = Filter();
		filter2.setFiltertext("");
		QVERIFY(filter2.cleared());
	}
	{ // empty
		auto filter3 = Filter();
		filter3.setFiltertext("");
		filter3.setMode(Filter::Mode::InvalidGenre);
		QVERIFY(filter3.cleared() == false);
	}
}

[[maybe_unused]] void LibraryFilterTest::testFilterLength() // NOLINT(readability-convert-member-functions-to-static)
{
	auto filter1 = Filter();
	auto filter2 = Filter();

	{ // too short searchstrings are invalid
		constexpr const auto MinimumSearchStringLength = 5;
		filter1.setFiltertext("abc");
		filter2.setFiltertext("def");
		QVERIFY(filter1.isEqual(filter2, MinimumSearchStringLength) == true);
	}

	{ // comparison is done as soon as filtertext is long enough
		constexpr const auto MinimumSearchStringLength = 5;
		filter1.setFiltertext("abcde");
		filter2.setFiltertext("defgh");
		QVERIFY(filter1.isEqual(filter2, MinimumSearchStringLength) == false);
	}

	{ // changing the length during lifetime of filter affects comparison
		constexpr const auto MinimumSearchStringLength = 6;
		QVERIFY(filter1.isEqual(filter2, MinimumSearchStringLength) == true);
	}
}

QTEST_GUILESS_MAIN(LibraryFilterTest)

#include "LibraryFilterTest.moc"
