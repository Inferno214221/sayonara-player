#include "SayonaraTest.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Set.h"

#include <algorithm>

class MetaDataTest : public SayonaraTest
{
	Q_OBJECT

public:
	MetaDataTest() :
		SayonaraTest("MetaDataTest")
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

	QVERIFY(md.is_equal(create_md()));
	QVERIFY(md.is_equal_deep(create_md()));

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

	MetaData md4 = std::move(md3);
	{
		UniqueId uid = md3.unique_id();
		QVERIFY(md4.is_equal(create_md()));
		QVERIFY(md4.is_equal_deep(create_md()));
		QVERIFY(md4.unique_id() == uid);
	}

	MetaData md5(std::move(md4));
	{
		UniqueId uid = md4.unique_id();
		QVERIFY(md5.is_equal(create_md()));
		QVERIFY(md5.is_equal_deep(create_md()));
		QVERIFY(md5.unique_id() == uid);
	}

	MetaData md6(std::move(md5));
	{
		md6.set_disabled( !md6.is_disabled() );
		QVERIFY(md6.is_equal(create_md()));
		QVERIFY(md6.is_equal_deep(create_md()) == false);
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
	MetaData md1 = create_md();
	MetaData md2(std::move(md1));

	QVERIFY(md2.is_equal_deep(create_md()));
}


QTEST_GUILESS_MAIN(MetaDataTest)

#include "MetaDataTest.moc"

