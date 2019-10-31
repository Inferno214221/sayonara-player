#include "SayonaraTest.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Set.h"

#include <algorithm>

class MetaDataTest : public Test::Base
{
	Q_OBJECT

public:
	MetaDataTest() :
		Test::Base("MetaDataTest")
	{}

	~MetaDataTest() override = default;

private slots:
	void copy_test();
	void genre_test();
	void private_test();
	void stream_test();
	void move_test();
};

static MetaData create_md()
{
	MetaData md("/path/to/my/file.mp3");
	md.set_title("Title");
	md.set_artist("Artist");
	md.set_album("Album");
	md.set_duration_ms(100000);
	md.set_filesize(1234567);
	md.set_id(5);
	md.set_artist_id(6);
	md.set_album_id(7);
	md.set_bitrate(320000);
	md.set_track_number(17);
	md.set_year(2014);
	md.set_extern(true);
	md.set_disabled(true);
	md.set_rating(Rating::Four);
	md.set_discnumber(2);
	md.set_disc_count(5);
	md.set_library_id(2);
	md.set_disabled(true);

	md.add_genre(Genre("Metal"));
	md.add_genre(Genre("Rock"));
	md.set_album_artist("Album artist", 14);

	return md;
}

void MetaDataTest::copy_test()
{
	MetaData md = create_md();
	MetaData md2 = md;
	{
		QVERIFY(md2.is_equal(create_md()));
		QVERIFY(md2.is_equal_deep(create_md()));
		QVERIFY(md2.unique_id() != md.unique_id());
	}

	MetaData md3(md2);
	{
		QVERIFY(md3.is_equal(create_md()));
		QVERIFY(md3.is_equal_deep(create_md()));
		QVERIFY(md3.unique_id() != md2.unique_id());
	}
}

void MetaDataTest::genre_test()
{
	MetaData md = create_md();
	QVERIFY( md.has_genre(Genre("Metal")));
	QVERIFY( md.has_genre(Genre("Rock")));
}

void MetaDataTest::private_test()
{
	MetaData md = create_md();
	QVERIFY( md.album_artist_id() == 14);
	QVERIFY( md.album_artist().compare("Album artist") == 0);
}

void MetaDataTest::stream_test()
{
	MetaData md = create_md();

	QVERIFY( md.radio_mode() == RadioMode::Off );

	md.set_filepath("http://path.to/my/stream");
	QVERIFY( md.radio_mode() == RadioMode::Station );
}

void MetaDataTest::move_test()
{
	MetaData md_orig = create_md();

	MetaData md1 = md_orig;
	UniqueId uid = md1.unique_id();

	// move md1 to md2
	MetaData md2(std::move(md1));
	QVERIFY(md2.is_equal_deep(md_orig));
	QVERIFY(md2.unique_id() == uid);

	// move md2 to md3
	MetaData md3 = std::move(md2);
	QVERIFY(md3.is_equal_deep(md_orig));
	QVERIFY(md3.unique_id() == uid);
}

QTEST_GUILESS_MAIN(MetaDataTest)

#include "MetaDataTest.moc"

