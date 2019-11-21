#include "SayonaraTest.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverFetchManager.h"

#include <QMap>

using Cover::Location;

class CoverLocationTest :
	public Test::Base
{
	Q_OBJECT

public:
	CoverLocationTest() :
		Test::Base("CoverLocationTest")
	{}

	~CoverLocationTest() override = default;

private slots:
	void test_copy();
};

void CoverLocationTest::test_copy()
{
	Location cl1 = Location::cover_location("AnAlbum", "AnArtist");
	cl1.set_search_term("some search term");
	QVERIFY(cl1.is_valid());
	QVERIFY(!cl1.hash().isEmpty());
	QVERIFY(!cl1.identifer().isEmpty());
	QVERIFY(!cl1.cover_path().isEmpty());
	QVERIFY(!cl1.to_string().isEmpty());
	QVERIFY(!cl1.search_term().isEmpty());
	QVERIFY(!cl1.search_urls().isEmpty());

	Location cl2 = cl1;
	QVERIFY(cl2.is_valid() == cl1.is_valid());
	QVERIFY(cl2.hash() == cl1.hash());
	QVERIFY(cl2.identifer() == cl1.identifer());
	QVERIFY(cl2.cover_path() == cl1.cover_path());
	QVERIFY(cl2.to_string() == cl1.to_string());
	QVERIFY(cl2.local_path() == cl1.local_path());
	QVERIFY(cl2.search_term() == cl1.search_term());

	Location cl3(cl1);
	QVERIFY(cl3.is_valid() == cl1.is_valid());
	QVERIFY(cl3.hash() == cl1.hash());
	QVERIFY(cl3.identifer() == cl1.identifer());
	QVERIFY(cl3.cover_path() == cl1.cover_path());
	QVERIFY(cl3.to_string() == cl1.to_string());
	QVERIFY(cl3.local_path() == cl1.local_path());
	QVERIFY(cl3.search_term() == cl1.search_term());
}


QTEST_GUILESS_MAIN(CoverLocationTest)

#include "CoverLocationTest.moc"

