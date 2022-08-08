#include "test/Common/SayonaraTest.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Library/Filter.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Algorithm.h"

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	int searchByGenre(const QString& searchString)
	{
		auto* db = DB::Connector::instance();
		DB::LibraryDatabase* libDb = db->libraryDatabase(0, 0);

		Library::Filter filter;
		filter.setMode(Library::Filter::Mode::Genre);
		filter.setFiltertext(searchString);

		MetaDataList tracks;
		libDb->getAllTracksBySearchString(filter, tracks);

		return tracks.count();
	}

	void updateSearchmode(const Library::SearchModeMask smm)
	{
		SetSetting(Set::Lib_SearchMode, smm);

		auto* db = DB::Connector::instance();
		auto* libDb = db->libraryDatabase(0, 0);

		libDb->updateSearchMode();
	}
}

class CissearchTest :
	public Test::Base
{
	Q_OBJECT

	public:
		CissearchTest() :
			Test::Base("CissearchTest")
		{
			auto* db = DB::Connector::instance();
			db->registerLibraryDatabase(0);
		}

	private slots:
		[[maybe_unused]] void uppercaseTest();
		[[maybe_unused]] void diacrticTest();
		[[maybe_unused]] void specialCharsTest();
		[[maybe_unused]] void fullMaskTest();
		[[maybe_unused]] void genreListTest();
};

[[maybe_unused]] void CissearchTest::uppercaseTest() // NOLINT(readability-convert-member-functions-to-static)
{
	using Library::convertSearchstring;
	const auto searchModeMask = +Library::SearchMode::CaseInsensitve;

	QVERIFY(convertSearchstring("ArTiSt", searchModeMask) == convertSearchstring("aRtIsT", searchModeMask));

	const auto searchModeMask2 = +Library::SearchMode::None;
	QVERIFY(convertSearchstring("ArTiSt", searchModeMask2) != convertSearchstring("aRtIsT", searchModeMask2));
}

[[maybe_unused]] void CissearchTest::diacrticTest() // NOLINT(readability-convert-member-functions-to-static)
{
	using Library::convertSearchstring;
	const auto searchModeMask = +Library::SearchMode::NoDiacriticChars;

	QVERIFY(convertSearchstring(QString::fromUtf8("string1ä"), searchModeMask) ==
	        convertSearchstring("string1a", searchModeMask));
	QVERIFY(convertSearchstring(QString::fromUtf8("striÖng2"), searchModeMask) ==
	        convertSearchstring("striOng2", searchModeMask));
	QVERIFY(convertSearchstring(QString::fromUtf8("strîArt3"), searchModeMask) ==
	        convertSearchstring("striArt3", searchModeMask));

	const auto searchModeMask2 = +Library::SearchMode::None;
	QVERIFY(convertSearchstring(QString::fromUtf8("string1ä"), searchModeMask2) !=
	        convertSearchstring("string1a", searchModeMask2));
	QVERIFY(convertSearchstring(QString::fromUtf8("striÖng2"), searchModeMask2) !=
	        convertSearchstring("striOng2", searchModeMask2));
	QVERIFY(convertSearchstring(QString::fromUtf8("strîArt3"), searchModeMask2) !=
	        convertSearchstring("striArt3", searchModeMask2));
}

[[maybe_unused]] void CissearchTest::specialCharsTest() // NOLINT(readability-convert-member-functions-to-static)
{
	using Library::convertSearchstring;
	const auto searchModeMask = +Library::SearchMode::NoSpecialChars;

	QVERIFY(convertSearchstring(QString::fromUtf8("soap&skin"), searchModeMask) ==
	        convertSearchstring("soap skin", searchModeMask));
	QVERIFY(convertSearchstring(QString::fromUtf8("guns 'n' roses"), searchModeMask) ==
	        convertSearchstring("gunsnroses", searchModeMask));
	QVERIFY(
		convertSearchstring(QString::fromUtf8("Billy Talent"), searchModeMask) ==
		convertSearchstring("Billy      Talent", searchModeMask));

	const auto searchModeMask2 = +Library::SearchMode::None;
	QVERIFY(convertSearchstring(QString::fromUtf8("soap&skin"), searchModeMask2) !=
	        convertSearchstring("soap skin", searchModeMask2));
	QVERIFY(
		convertSearchstring(QString::fromUtf8("guns 'n' roses"), searchModeMask2) !=
		convertSearchstring("gunsnroses", searchModeMask2));
	QVERIFY(
		convertSearchstring(QString::fromUtf8("Billy Talent"), searchModeMask2) !=
		convertSearchstring("Billy      Talent", searchModeMask2));
}

[[maybe_unused]] void CissearchTest::fullMaskTest() // NOLINT(readability-convert-member-functions-to-static)
{
	using Library::convertSearchstring;
	const auto searchModeMask = +Library::SearchMode::NoDiacriticChars |
	                            +Library::SearchMode::CaseInsensitve |
	                            +Library::SearchMode::NoSpecialChars;

	QVERIFY(convertSearchstring(QString::fromUtf8("soap&skin"), searchModeMask) ==
	        convertSearchstring("söäp sKin", searchModeMask));
	QVERIFY(
		convertSearchstring(QString::fromUtf8("Güns    'n' röses"), searchModeMask) ==
		convertSearchstring("guns n' roses", searchModeMask));
	QVERIFY(convertSearchstring(QString::fromUtf8("Bîlly Tälent"), searchModeMask) ==
	        convertSearchstring("billytalent", searchModeMask));
}

[[maybe_unused]] void
CissearchTest::genreListTest() // NOLINT(readability-function-cognitive-complexity,readability-convert-member-functions-to-static)
{
	auto md = MetaData {};
	md.setArtist("Artist");
	md.setAlbum("Album");
	md.setTitle("Title");
	md.setFilepath("/path/to/nowhere.mp3");

	const auto genreNames = QStringList
		{
			"1Pop",
			"2poP",
			"3Rock",
			"4psy rock",
			"5hip-hop",
			"6Hip Hop"
		};

	for(const auto& name: genreNames)
	{
		md.addGenre(Genre(name));
	}

	auto* db = DB::Connector::instance();
	auto* libDb = db->libraryDatabase(0, 0);
	const auto tracks = MetaDataList {md};
	const auto success = libDb->storeMetadata(tracks);
	QVERIFY(success == true);
	{
		MetaDataList tracksTmp;
		libDb->getAllTracks(tracksTmp);
		QVERIFY(tracksTmp.size() == 1);
		QVERIFY(tracksTmp.first().title() == "Title");
	}

	auto genreList = md.genresToList();
	genreList.sort();

	{
		const auto searchModeMask = 0;
		updateSearchmode(searchModeMask);

		const auto cis = Library::convertSearchstring(genreList.join(","), searchModeMask);
		QVERIFY(cis == "1Pop,2poP,3Rock,4psy rock,5hip-hop,6Hip Hop");

		auto c = searchByGenre("Hip Hop");
		QVERIFY(c == 1);

		c = searchByGenre("Hiphop");
		QVERIFY(c == 0);

		c = searchByGenre("hiphop");
		QVERIFY(c == 0);

		c = searchByGenre("hip-hop");
		QVERIFY(c == 1);

		c = searchByGenre("hip hop");
		QVERIFY(c == 0);
	}

	{
		const auto searchModeMask = +Library::SearchMode::NoSpecialChars;
		updateSearchmode(searchModeMask);

		const auto cis = Library::convertSearchstring(genreList.join(","), searchModeMask);
		QVERIFY(cis == "1Pop2poP3Rock4psyrock5hiphop6HipHop");

		auto c = searchByGenre("Hip Hop");
		QVERIFY(c == 1);

		c = searchByGenre("Hiphop");
		QVERIFY(c == 0);

		c = searchByGenre("hiphop");
		QVERIFY(c == 1);

		c = searchByGenre("hip-hop");
		QVERIFY(c == 1);

		c = searchByGenre("hip Hop");
		QVERIFY(c == 0);
	}

	{
		const auto searchModeMask = +Library::SearchMode::CaseInsensitve;
		updateSearchmode(searchModeMask);

		const auto cis = Library::convertSearchstring(genreList.join(","), searchModeMask);
		QVERIFY(cis == "1pop,2pop,3rock,4psy rock,5hip-hop,6hip hop");

		auto c = searchByGenre("Hip Hop");
		QVERIFY(c == 1);

		c = searchByGenre("Hiphop");
		QVERIFY(c == 0);

		c = searchByGenre("hiphop");
		QVERIFY(c == 0);

		c = searchByGenre("hip-hop");
		QVERIFY(c == 1);

		c = searchByGenre("hip Hop");
		QVERIFY(c == 1);
	}

	{
		constexpr const auto searchModeMask = +Library::SearchMode::CaseInsensitve |
		                                      +Library::SearchMode::NoSpecialChars;
		updateSearchmode(searchModeMask);

		const auto cis = Library::convertSearchstring(genreList.join(","), searchModeMask);
		QVERIFY(cis == "1pop2pop3rock4psyrock5hiphop6hiphop");

		auto c = searchByGenre("Hip Hop");
		QVERIFY(c == 1);

		c = searchByGenre("Hiphop");
		QVERIFY(c == 1);

		c = searchByGenre("hiphop");
		QVERIFY(c == 1);

		c = searchByGenre("hip-hop");
		QVERIFY(c == 1);

		c = searchByGenre("hip Hop");
		QVERIFY(c == 1);
	}
}

QTEST_GUILESS_MAIN(CissearchTest)

#include "CissearchTest.moc"
