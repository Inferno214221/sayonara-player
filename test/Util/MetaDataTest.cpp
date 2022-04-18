#include "test/Common/SayonaraTest.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Set.h"

#include <algorithm>
#include <array>
#include <utility>

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
	void setRadioStationTest();
};

static MetaData create_md()
{
	MetaData md("/path/to/my/file.mp3");
	md.setTitle("Title");
	md.setArtist("Artist");
	md.setAlbum("Album");
	md.setDurationMs(100000);
	md.setFilesize(1234567);
	md.setId(5);
	md.setArtistId(6);
	md.setAlbumId(7);
	md.setBitrate(320000);
	md.setTrackNumber(17);
	md.setYear(2014);
	md.setExtern(true);
	md.setDisabled(true);
	md.setRating(Rating::Four);
	md.setDiscnumber(2);
	md.setDiscCount(5);
	md.setLibraryid(2);
	md.setDisabled(true);

	md.addGenre(Genre("Metal"));
	md.addGenre(Genre("Rock"));
	md.setAlbumArtist("Album artist", 14);

	return md;
}

void MetaDataTest::copy_test()
{
	MetaData md = create_md();
	MetaData md2 = md;
	{
		QVERIFY(md2.isEqual(create_md()));
		QVERIFY(md2.isEqualDeep(create_md()));
		QVERIFY(md2.uniqueId() != md.uniqueId());
	}

	MetaData md3(md2);
	{
		QVERIFY(md3.isEqual(create_md()));
		QVERIFY(md3.isEqualDeep(create_md()));
		QVERIFY(md3.uniqueId() != md2.uniqueId());
	}
}

void MetaDataTest::genre_test()
{
	MetaData md = create_md();
	QVERIFY( md.hasGenre(Genre("Metal")));
	QVERIFY( md.hasGenre(Genre("Rock")));
}

void MetaDataTest::private_test()
{
	MetaData md = create_md();
	QVERIFY( md.albumArtistId() == 14);
	QVERIFY( md.albumArtist().compare("Album artist") == 0);
}

struct Result {
	QString station;
	QString stationName;
};

void MetaDataTest::stream_test()
{
	auto track = create_md();
	QVERIFY(track.radioMode() == RadioMode::Off);

	track.setFilepath("http://path.to/my/stream");
	QVERIFY(track.radioMode() == RadioMode::Station);
}

void MetaDataTest::setRadioStationTest()
{
	const auto testCases = std::array {
		std::tuple {"", "", Result {"", ""}},
		std::tuple {"http://path.to/stream", "", Result {"http://path.to/stream", "path.to"}},
		std::tuple {"http://path.to/stream", "Name", Result {"http://path.to/stream", "Name"}},
		std::tuple {"path.to/stream", "", Result {"path.to/stream", "path.to/stream"}},
		std::tuple {"path.to/stream", "Name", Result {"path.to/stream", "Name"}}
	};

	for(const auto& testCase: testCases)
	{
		const auto&[url, stationName, result] = testCase;
		const auto&[expectedStation, expectedStationName] = result;

		auto track = MetaData {};
		track.setRadioStation(url, stationName);

		const auto station = track.radioStation();
		const auto radioStationName = track.radioStationName();

		QVERIFY(track.radioStation() == expectedStation);
		QVERIFY(track.radioStationName() == expectedStationName);

		QVERIFY(track.artist() == track.radioStation());
		QVERIFY(track.title() == track.radioStationName());
		QVERIFY(track.album() == track.title());
	}
}

void MetaDataTest::move_test()
{
	MetaData md_orig = create_md();

	MetaData md1 = md_orig;
	UniqueId uid = md1.uniqueId();

	// move md1 to md2
	MetaData md2(std::move(md1));
	QVERIFY(md2.isEqualDeep(md_orig));
	QVERIFY(md2.uniqueId() == uid);

	// move md2 to md3
	MetaData md3 = std::move(md2);
	QVERIFY(md3.isEqualDeep(md_orig));
	QVERIFY(md3.uniqueId() == uid);
}

QTEST_GUILESS_MAIN(MetaDataTest)

#include "MetaDataTest.moc"

