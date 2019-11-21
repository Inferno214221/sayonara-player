#include "SayonaraTest.h"

#include "Utils/MetaData/Album.h"

class AlbumTest : public Test::Base
{
	Q_OBJECT

public:
	AlbumTest() :
		Test::Base("MDAlbumTest")
	{}

	~AlbumTest() override = default;

private slots:
	void test();
};

void AlbumTest::copy_test()
{
	Album a1;

	a1.set_album_artists({"Album3"});
	a1.set_artists({"artist1", "artist2"});
	a1.set_cover_download_urls({"http://bla.jpg", "https://bla.jpg"});
	a1.set_db_id(2);
	a1.set_discnumbers({0,3,2,3});
	a1.set_duration_sec(150);
	a1.set_id(4);
	a1.set_name("Album1");
	a1.set_path_hint({"/some/hint/to/path"});
	a1.set_songcount(4);
	a1.set_rating(Rating::Three);
	a1.set_year(2005);

	QVERIFY(a1.db_id() == 2);
	QVERIFY(a1.duration_sec() == 150);
	QVERIFY(a1.id() == 4);
	QVERIFY(a1.name() == "Album1");
	QVERIFY(a1.songcount() == 4);
	QVERIFY(a1.rating() == Rating::Three);
	QVERIFY(a1.year() == 2005);
	QVERIFY(a1.is_sampler() == true);
	QVERIFY(a1.disccount() == 3);

	Album a2 = a1;
	QVERIFY(a2 == a1);
	a2.set_songcount(100);
	QVERIFY(!(a2 == a1));

	Album a3(a1);
	QVERIFY(a3 == a1);

	Album a4 = std::move(a3);
	QVERIFY(a4 == a1);

	Album a5(std::move(a4));
	QVERIFY(a5 == a1);
}
