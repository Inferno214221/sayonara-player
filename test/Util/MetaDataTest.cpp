#include <QObject>
#include <QTest>
#include <QDebug>
#include <QString>

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Set.h"

#include <algorithm>

class MetaDataTest : public QObject
{
	Q_OBJECT

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
	md.duration_ms = 100000;
	md.filesize = 1234567;
	md.id = 5;
	md.artist_id = 6;
	md.album_id = 7;
	md.bitrate = 320000;
	md.track_num = 17;
	md.year = 2014;
	md.played = true;
	md.is_extern = true;
	md.pl_playing = true;
	md.is_disabled = true;
	md.rating = Rating::Four;
	md.discnumber = 2;
	md.n_discs = 5;
	md.library_id = 2;

	md.add_genre(Genre("Metal"));
	md.add_genre(Genre("Rock"));
	md.set_album_artist("Album artist", 14);

	return md;
}

void MetaDataTest::copy_test()
{
	qDebug() << "Copy Test";

	MetaData md("/path/to/my/file.mp3");
	md.set_title("Title");
	md.set_artist("Artist");
	md.set_album("Album");
	md.duration_ms = 100000;
	md.filesize = 1234567;
	md.id = 5;
	md.artist_id = 6;
	md.album_id = 7;
	md.bitrate = 320000;
	md.track_num = 17;
	md.year = 2014;
	md.played = true;
	md.is_extern = true;
	md.pl_playing = true;
	md.is_disabled = true;
	md.rating = Rating::Four;
	md.discnumber = 2;
	md.n_discs = 5;
	md.library_id = 2;

	md.add_genre(Genre("Metal"));
	md.add_genre(Genre("Rock"));

	md.set_album_artist("Album artist", 14);

	QVERIFY(md.is_equal(create_md()));
	QVERIFY(md.is_equal_deep(create_md()));
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

