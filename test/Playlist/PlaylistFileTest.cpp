#include "PlaylistTestUtils.h"

#include "test/Common/SayonaraTest.h"

#include "Utils/Parser/M3UParser.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"

// access working directory with Test::Base::tempPath("somefile.txt");

using Test::Playlist::PathTrackMap;

namespace
{
	constexpr const auto ExpectedPlaylist = R"(#EXTM3U
#EXTINF:100,Artist1 - Title1
%1path/mp3test.mp3

#EXTINF:200,Artist2 - Title2
%1path/to/mp3test.mp3

#EXTINF:300,Artist3 - Title3
%1path/to/somewhere/mp3test.mp3

#EXTINF:400,Artist4 - Title4
%1path/to/somewhere/else/mp3test.mp3

#EXTINF:500,Artist5 - Title5
%1path/to/another/mp3test.mp3

#EXTINF:600,Artist6 - Title6
%1path/to/another/dir/mp3test.mp3
)";
}

class PlaylistFileTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistFileTest() :
			Test::Base("PlaylistFileTest")
		{
			const auto basePath = Test::Base::tempPath();
			m_tracks = Test::Playlist::createTrackFiles(basePath);
		}

	private slots:
		void parseStandardM3U();
		void parseExtendedM3U();
		void writeM3uAbsolute();
		void writeM3uRelative();

	private:
		PathTrackMap m_tracks;
};

void PlaylistFileTest::parseStandardM3U()
{
	QString content = R"m3u(
	file1.mp3

	file2.ogg
	<file3>
    # don't parse this
	<file4>
	)m3u";

	content.replace("<file3>", this->tempPath("file3.flac"));
	content.replace("<file4>", this->tempPath("file4.aac"));

	const auto filename = this->tempPath("standard.m3u");
	const auto success = Util::File::writeFile(content.toUtf8(), filename);
	QVERIFY(success);

	Util::File::writeFile(QByteArray(), this->tempPath("file1.mp3"));
	Util::File::writeFile(QByteArray(), this->tempPath("file2.ogg"));
	Util::File::writeFile(QByteArray(), this->tempPath("file3.flac"));
	Util::File::writeFile(QByteArray(), this->tempPath("file4.aac"));

	auto parser = M3UParser(filename);
	const auto tracks = parser.tracks();

	QVERIFY(tracks.size() == 4);
	QVERIFY(tracks[0].filepath() == this->tempPath("file1.mp3"));
	QVERIFY(tracks[1].filepath() == this->tempPath("file2.ogg"));
	QVERIFY(tracks[2].filepath() == this->tempPath("file3.flac"));
	QVERIFY(tracks[3].filepath() == this->tempPath("file4.aac"));
}

void PlaylistFileTest::parseExtendedM3U()
{
	QString content = R"m3u(
	#EXTM3U

	#EXTINF:221,Queen - Bohemian Rhapsody
	file1.mp3

	#EXTINF:473, Dire Straits - Walk Of Life
	file2.ogg
	# This is a comment
	#EXTINF:264 ,冨田勲 - Boléro
	<file3>
	#EXTINF: 504,  Bob Marley - Buffalo Soldier

<file4>
	)m3u";

	content.replace("<file3>", this->tempPath("file3.flac"));
	content.replace("<file4>", this->tempPath("file4.aac"));

	const auto filename = this->tempPath("extended.m3u");
	const auto success = Util::File::writeFile(content.toUtf8(), filename);
	QVERIFY(success);

	Util::File::writeFile(QByteArray(), this->tempPath("file1.mp3"));
	Util::File::writeFile(QByteArray(), this->tempPath("file2.ogg"));
	Util::File::writeFile(QByteArray(), this->tempPath("file3.flac"));
	Util::File::writeFile(QByteArray(), this->tempPath("file4.aac"));

	auto parser = M3UParser(filename);
	const auto tracks = parser.tracks();

	QVERIFY(tracks.size() == 4);

	QVERIFY(tracks[0].durationMs() == 221'000);
	QVERIFY(tracks[0].artist() == "Queen");
	QVERIFY(tracks[0].title() == "Bohemian Rhapsody");
	QVERIFY(tracks[0].filepath() == this->tempPath("file1.mp3"));

	QVERIFY(tracks[1].durationMs() == 473'000);
	QVERIFY(tracks[1].artist() == "Dire Straits");
	QVERIFY(tracks[1].title() == "Walk Of Life");
	QVERIFY(tracks[1].filepath() == this->tempPath("file2.ogg"));

	QVERIFY(tracks[2].durationMs() == 264'000);
	QVERIFY(tracks[2].artist() == QString::fromUtf8("冨田勲"));
	QVERIFY(tracks[2].title() == QString::fromUtf8("Boléro"));
	QVERIFY(tracks[2].filepath() == this->tempPath("file3.flac"));

	QVERIFY(tracks[3].durationMs() == 504'000);
	QVERIFY(tracks[3].artist() == "Bob Marley");
	QVERIFY(tracks[3].title() == "Buffalo Soldier");
	QVERIFY(tracks[3].filepath() == this->tempPath("file4.aac"));
}

void PlaylistFileTest::writeM3uAbsolute()
{
	MetaDataList tracks;
	for(const auto& [path, track] : m_tracks)
	{
		tracks << track;
	}

	const auto playlistName = Base::tempPath("absolutePlaylist.m3u");
	M3UParser::saveM3UPlaylist(playlistName, tracks, false);

	QString content;
	Util::File::readFileIntoString(playlistName, content);

	const auto absoluteExpectedPlaylist = QString(ExpectedPlaylist).arg(Base::tempPath() + '/');
	QVERIFY(content == absoluteExpectedPlaylist);
}

void PlaylistFileTest::writeM3uRelative()
{
	MetaDataList tracks;
	for(const auto& [path, track] : m_tracks)
	{
		tracks << track;
	}

	const auto playlistName = Base::tempPath("relativePlaylist.m3u");
	M3UParser::saveM3UPlaylist(playlistName, tracks, true);

	QString content;
	Util::File::readFileIntoString(playlistName, content);

	const auto relativeExpectedPlaylist = QString(ExpectedPlaylist).arg("");
	QVERIFY(content == relativeExpectedPlaylist);
}

QTEST_GUILESS_MAIN(PlaylistFileTest)

#include "PlaylistFileTest.moc"
