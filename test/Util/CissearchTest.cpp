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
		void uppercaseTest();
		void diacrticTest();
		void specialCharsTest();
		void fullMaskTest();

		void genreListTest();

	private:
		Library::SearchModeMask mSMM;

		int searchByGenre(const QString& searchString)
		{
			auto* db = DB::Connector::instance();
			DB::LibraryDatabase* libDb = db->libraryDatabase(0, 0);

			Library::Filter filter;
			filter.setMode(Library::Filter::Mode::Genre);
			filter.setFiltertext(searchString, this->mSMM);

			MetaDataList tracks;
			libDb->getAllTracksBySearchString(filter, tracks);

			return tracks.count();
		}

		void updateSearchmode(Library::SearchModeMask smm)
		{
			this->mSMM = smm;

			auto* db = DB::Connector::instance();
			DB::LibraryDatabase* libDb = db->libraryDatabase(0, 0);

			Settings::instance()->set<Set::Lib_SearchMode>(smm);
			libDb->updateSearchMode();
		}
};

void CissearchTest::uppercaseTest()
{
	using Library::Utils::convertSearchstring;
	const auto smm = Library::SearchMode::CaseInsensitve;
	QVERIFY(convertSearchstring("ArTiSt", smm) == convertSearchstring("aRtIsT", smm));

	const auto smm2 = Library::SearchMode::None;
	QVERIFY(convertSearchstring("ArTiSt", smm2) != convertSearchstring("aRtIsT", smm2));
}

void CissearchTest::diacrticTest()
{
	using Library::Utils::convertSearchstring;
	const auto smm = Library::SearchMode::NoDiacriticChars;

	QVERIFY(convertSearchstring(QString::fromUtf8("string1ä"), smm) == convertSearchstring("string1a", smm));
	QVERIFY(convertSearchstring(QString::fromUtf8("striÖng2"), smm) == convertSearchstring("striOng2", smm));
	QVERIFY(convertSearchstring(QString::fromUtf8("strîArt3"), smm) == convertSearchstring("striArt3", smm));

	const auto smm2 = Library::SearchMode::None;
	QVERIFY(convertSearchstring(QString::fromUtf8("string1ä"), smm2) != convertSearchstring("string1a", smm2));
	QVERIFY(convertSearchstring(QString::fromUtf8("striÖng2"), smm2) != convertSearchstring("striOng2", smm2));
	QVERIFY(convertSearchstring(QString::fromUtf8("strîArt3"), smm2) != convertSearchstring("striArt3", smm2));
}

void CissearchTest::specialCharsTest()
{
	using Library::Utils::convertSearchstring;
	const auto smm = Library::SearchMode::NoSpecialChars;

	QVERIFY(convertSearchstring(QString::fromUtf8("soap&skin"), smm) == convertSearchstring("soap skin", smm));
	QVERIFY(convertSearchstring(QString::fromUtf8("guns 'n' roses"), smm) == convertSearchstring("gunsnroses", smm));
	QVERIFY(
		convertSearchstring(QString::fromUtf8("Billy Talent"), smm) == convertSearchstring("Billy      Talent", smm));

	const auto smm2 = Library::SearchMode::None;
	QVERIFY(convertSearchstring(QString::fromUtf8("soap&skin"), smm2) != convertSearchstring("soap skin", smm2));
	QVERIFY(
		convertSearchstring(QString::fromUtf8("guns 'n' roses"), smm2) != convertSearchstring("gunsnroses", smm2));
	QVERIFY(
		convertSearchstring(QString::fromUtf8("Billy Talent"), smm2) != convertSearchstring("Billy      Talent", smm2));
}

void CissearchTest::fullMaskTest()
{
	using Library::Utils::convertSearchstring;
	const Library::SearchModeMask smm = Library::SearchMode::NoDiacriticChars | Library::SearchMode::CaseInsensitve |
	                                    Library::SearchMode::NoSpecialChars;

	QVERIFY(convertSearchstring(QString::fromUtf8("soap&skin"), smm) == convertSearchstring("söäp sKin", smm));
	QVERIFY(
		convertSearchstring(QString::fromUtf8("Güns    'n' röses"), smm) == convertSearchstring("guns n' roses", smm));
	QVERIFY(convertSearchstring(QString::fromUtf8("Bîlly Tälent"), smm) == convertSearchstring("billytalent", smm));
}

void CissearchTest::genreListTest()
{
	MetaData md;
	md.setArtist("Artist");
	md.setAlbum("Album");
	md.setTitle("Title");
	md.setFilepath("/path/to/nowhere.mp3");

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

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* libDb = db->libraryDatabase(0, 0);
	MetaDataList tracks {md};
	bool success = libDb->storeMetadata(tracks);
	QVERIFY(success == true);
	{
		MetaDataList tracksTmp;
		libDb->getAllTracks(tracksTmp);
		QVERIFY(tracksTmp.size() == 1);
		QVERIFY(tracksTmp.first().title() == "Title");
	}

	QStringList genreList = md.genresToList();
	genreList.sort();

	{
		Library::SearchModeMask smm = 0;
		this->updateSearchmode(smm);

		QString cis = Library::Utils::convertSearchstring(genreList.join(","), smm);
		QVERIFY(cis == "1Pop,2poP,3Rock,4psy rock,5hip-hop,6Hip Hop");

		int c;
		c = this->searchByGenre("Hip Hop");
		QVERIFY(c == 1);

		c = this->searchByGenre("Hiphop");
		QVERIFY(c == 0);

		c = this->searchByGenre("hiphop");
		QVERIFY(c == 0);

		c = this->searchByGenre("hip-hop");
		QVERIFY(c == 1);

		c = this->searchByGenre("hip hop");
		QVERIFY(c == 0);
	}

	{
		Library::SearchModeMask smm = Library::SearchMode::NoSpecialChars;
		this->updateSearchmode(smm);

		QString cis = Library::Utils::convertSearchstring(genreList.join(","), smm);
		QVERIFY(cis == "1Pop2poP3Rock4psyrock5hiphop6HipHop");

		int c;
		c = this->searchByGenre("Hip Hop");
		QVERIFY(c == 1);

		c = this->searchByGenre("Hiphop");
		QVERIFY(c == 0);

		c = this->searchByGenre("hiphop");
		QVERIFY(c == 1);

		c = this->searchByGenre("hip-hop");
		QVERIFY(c == 1);

		c = this->searchByGenre("hip Hop");
		QVERIFY(c == 0);
	}

	{
		Library::SearchModeMask smm = Library::SearchMode::CaseInsensitve;
		this->updateSearchmode(smm);

		QString cis = Library::Utils::convertSearchstring(genreList.join(","), smm);
		QVERIFY(cis == "1pop,2pop,3rock,4psy rock,5hip-hop,6hip hop");

		int c;
		c = this->searchByGenre("Hip Hop");
		QVERIFY(c == 1);

		c = this->searchByGenre("Hiphop");
		QVERIFY(c == 0);

		c = this->searchByGenre("hiphop");
		QVERIFY(c == 0);

		c = this->searchByGenre("hip-hop");
		QVERIFY(c == 1);

		c = this->searchByGenre("hip Hop");
		QVERIFY(c == 1);
	}

	{
		Library::SearchModeMask smm = Library::SearchMode::CaseInsensitve | Library::SearchMode::NoSpecialChars;
		this->updateSearchmode(smm);

		QString cis = Library::Utils::convertSearchstring(genreList.join(","), smm);
		QVERIFY(cis == "1pop2pop3rock4psyrock5hiphop6hiphop");

		int c;
		c = this->searchByGenre("Hip Hop");
		QVERIFY(c == 1);

		c = this->searchByGenre("Hiphop");
		QVERIFY(c == 1);

		c = this->searchByGenre("hiphop");
		QVERIFY(c == 1);

		c = this->searchByGenre("hip-hop");
		QVERIFY(c == 1);

		c = this->searchByGenre("hip Hop");
		QVERIFY(c == 1);
	}
}

QTEST_GUILESS_MAIN(CissearchTest)

#include "CissearchTest.moc"
