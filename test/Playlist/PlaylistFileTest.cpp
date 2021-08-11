#include "SayonaraTest.h"
#include "Utils/Parser/M3UParser.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
// access working directory with Test::Base::tempPath("somefile.txt");


class PlaylistFileTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistFileTest() :
			Test::Base("PlaylistFileTest") {}

	private slots:
		void m3u();
};

void PlaylistFileTest::m3u()
{
	QString content = R"m3u(
	#EXTM3U
	#EXTINF:221,Queen - Bohemian Rhapsody
	file1.mp3
	#EXTINF:473, Dire Straits - Walk Of Life
	file2.ogg
	#EXTINF:264 ,冨田勲 - Boléro
	<file3>
	#EXTINF: 504,  Bob Marley - Buffalo Soldier
	<file4>
	)m3u";

	content.replace("<file3>", this->tempPath("file3.flac"));
	content.replace("<file4>", this->tempPath("file4.aac"));

	const QString filename(this->tempPath("playlist.m3u"));
	bool success = Util::File::writeFile(content.toUtf8(), filename);
	QVERIFY(success);

	Util::File::writeFile(QByteArray(), this->tempPath("file1.mp3"));
	Util::File::writeFile(QByteArray(), this->tempPath("file2.ogg"));
	Util::File::writeFile(QByteArray(), this->tempPath("file3.flac"));
	Util::File::writeFile(QByteArray(), this->tempPath("file4.aac"));

	auto parser = M3UParser(filename);
	auto tracks = parser.tracks();

	QVERIFY(tracks.size() == 4);

	QVERIFY(tracks[0].durationMs() == 221000);
	QVERIFY(tracks[0].artist() == "Queen");
	QVERIFY(tracks[0].title() == "Bohemian Rhapsody");
	QVERIFY(tracks[0].filepath() == this->tempPath("file1.mp3"));

	QVERIFY(tracks[1].durationMs() == 473000);
	QVERIFY(tracks[1].artist() == "Dire Straits");
	QVERIFY(tracks[1].title() == "Walk Of Life");
	QVERIFY(tracks[1].filepath() == this->tempPath("file2.ogg"));

	QVERIFY(tracks[2].durationMs() == 264000);
	QVERIFY(tracks[2].artist() == QString::fromUtf8("冨田勲"));
	QVERIFY(tracks[2].title() == QString::fromUtf8("Boléro"));
	QVERIFY(tracks[2].filepath() == this->tempPath("file3.flac"));

	QVERIFY(tracks[3].durationMs() == 504000);
	QVERIFY(tracks[3].artist() == "Bob Marley");
	QVERIFY(tracks[3].title() == "Buffalo Soldier");
	QVERIFY(tracks[3].filepath() == this->tempPath("file4.aac"));
}


QTEST_GUILESS_MAIN(PlaylistFileTest)
#include "PlaylistFileTest.moc"
