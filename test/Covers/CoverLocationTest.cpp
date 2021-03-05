#include "SayonaraTest.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverFetchManager.h"
#include "Utils/FileUtils.h"

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
	void testCopy();
};

void CoverLocationTest::testCopy()
{
	auto cl1 = Location::coverLocation("AnAlbum", "AnArtist");
	cl1.setSearchTerm("some search term");
	QVERIFY(cl1.isValid());
	QVERIFY(!cl1.hash().isEmpty());
	QVERIFY(!cl1.identifer().isEmpty());
	QVERIFY(!cl1.hashPath().isEmpty());
	QVERIFY(!cl1.toString().isEmpty());
	QVERIFY(!cl1.searchTerm().isEmpty());
	QVERIFY(!cl1.searchUrls().isEmpty());

	QVERIFY(Util::File::isImageFile(cl1.alternativePath()));

	auto cl2 = cl1;
	QVERIFY(cl2.isValid() == cl1.isValid());
	QVERIFY(cl2.hash() == cl1.hash());
	QVERIFY(cl2.identifer() == cl1.identifer());
	QVERIFY(cl2.hashPath() == cl1.hashPath());
	QVERIFY(cl2.toString() == cl1.toString());
	QVERIFY(cl2.localPath() == cl1.localPath());
	QVERIFY(cl2.searchTerm() == cl1.searchTerm());

	auto cl3 = Location(cl1);
	QVERIFY(cl3.isValid() == cl1.isValid());
	QVERIFY(cl3.hash() == cl1.hash());
	QVERIFY(cl3.identifer() == cl1.identifer());
	QVERIFY(cl3.hashPath() == cl1.hashPath());
	QVERIFY(cl3.toString() == cl1.toString());
	QVERIFY(cl3.localPath() == cl1.localPath());
	QVERIFY(cl3.searchTerm() == cl1.searchTerm());

	const auto cl4 = std::move(cl2);
	QVERIFY(cl4.isValid() == cl1.isValid());
	QVERIFY(cl4.hash() == cl1.hash());
	QVERIFY(cl4.identifer() == cl1.identifer());
	QVERIFY(cl4.hashPath() == cl1.hashPath());
	QVERIFY(cl4.toString() == cl1.toString());
	QVERIFY(cl4.localPath() == cl1.localPath());
	QVERIFY(cl4.searchTerm() == cl1.searchTerm());

	const auto cl5 = Location(std::move(cl3));
	QVERIFY(cl5.isValid() == cl1.isValid());
	QVERIFY(cl5.hash() == cl1.hash());
	QVERIFY(cl5.identifer() == cl1.identifer());
	QVERIFY(cl5.hashPath() == cl1.hashPath());
	QVERIFY(cl5.toString() == cl1.toString());
	QVERIFY(cl5.localPath() == cl1.localPath());
	QVERIFY(cl5.searchTerm() == cl1.searchTerm());
}

QTEST_GUILESS_MAIN(CoverLocationTest)

#include "CoverLocationTest.moc"

