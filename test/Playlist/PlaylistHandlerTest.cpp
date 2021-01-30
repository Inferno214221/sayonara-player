#include "SayonaraTest.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/Playlist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/FileUtils.h"
// access working directory with Test::Base::tempPath("somefile.txt");

class PlaylistHandlerTest : 
    public Test::Base
{
    Q_OBJECT

    public:
        PlaylistHandlerTest() :
            Test::Base("PlaylistHandlerTest")
        {}

    private slots:
        void createTest();
        void closeTest();
        void currentIndexTest();
        void activeIndexTest();
};

void PlaylistHandlerTest::createTest()
{
	auto* plh = Playlist::Handler::instance();
	QVERIFY(plh->count() == 0);
	QVERIFY(plh->activeIndex() == -1);

	auto index = plh->createEmptyPlaylist(false);
	QVERIFY(plh->playlist(index)->index() == index);
	QVERIFY(index == 0);
	QVERIFY(plh->count() == 1);

	index = plh->createEmptyPlaylist(false);
	QVERIFY(plh->playlist(index)->index() == index);
	QVERIFY(index == 1);
	QVERIFY(plh->count() == 2);

	index = plh->createEmptyPlaylist(true);
	QVERIFY(plh->playlist(index)->index() == index);
	QVERIFY(index == 1);
	QVERIFY(plh->count() == 2);

	plh->shutdown();
}

void PlaylistHandlerTest::closeTest()
{
	auto* plh = Playlist::Handler::instance();
	plh->createEmptyPlaylist(false);
	plh->createEmptyPlaylist(false);
	QVERIFY(plh->count() == 2);

	plh->closePlaylist(-4);
	QVERIFY(plh->count() == 2);

	plh->closePlaylist(0);
	QVERIFY(plh->count() == 1);
	QVERIFY(plh->playlist(0)->index() == 0);
	QVERIFY(plh->activeIndex() == 0);

	for(int i=0; i<50; i++)
	{
		plh->closePlaylist(0);
		QVERIFY(plh->count() == 1);
		QVERIFY(plh->playlist(0)->index() == 0);
		QVERIFY(plh->activeIndex() == 0);
	}

	plh->shutdown();
}

void PlaylistHandlerTest::currentIndexTest()
{
	auto* plh = Playlist::Handler::instance();

	QVERIFY(plh->currentIndex() == -1);
	plh->setCurrentIndex(20);
	QVERIFY(plh->currentIndex() == -1);

	plh->createEmptyPlaylist(false);
	QVERIFY(plh->currentIndex() == 0);

	plh->createEmptyPlaylist(false);
	QVERIFY(plh->currentIndex() == 1);

	plh->setCurrentIndex(5); // invalid current index
	QVERIFY(plh->currentIndex() == 1);

	plh->setCurrentIndex(0); // valid current index
	QVERIFY(plh->currentIndex() == 0);

	plh->setCurrentIndex(1);
	plh->closePlaylist(1); // delete current index pl
	QVERIFY(plh->currentIndex() == 0);

	plh->closePlaylist(0); // delete last playlist
	QVERIFY(plh->currentIndex() == 0);

	plh->shutdown();
}

void PlaylistHandlerTest::activeIndexTest()
{
	auto* plh = Playlist::Handler::instance();
	QVERIFY(plh->activeIndex() == -1); // empty playlist handler

	plh->createEmptyPlaylist();
	QVERIFY(plh->activeIndex() == 0); // one playlist

	plh->createEmptyPlaylist();
	QVERIFY(plh->activeIndex() == 1); // two playlists

	plh->closePlaylist(0);
	QVERIFY(plh->activeIndex() == 0); // one playlist

	plh->closePlaylist(0);
	QVERIFY(plh->activeIndex() == 0); // one playlist

	plh->setCurrentIndex(0);

	MetaDataList tracks;
	for(int i=0; i<10; i++)
	{
		// file must exist
		const auto filename = Test::Base::tempPath(QString("file%1.mp").arg(i));
		Util::File::writeFile(QByteArray{}, filename);

		tracks << MetaData{filename};
	}

	auto index = plh->createPlaylist(tracks, "new-playlist");
	QVERIFY(index == 1);

	auto playlist = plh->playlist(index);

	playlist->changeTrack(4, 0);
	playlist->play();
	QVERIFY(plh->activeIndex() == index);

	playlist->stop();
	QVERIFY(plh->activeIndex() == plh->currentIndex());

	plh->shutdown();
}

QTEST_GUILESS_MAIN(PlaylistHandlerTest)
#include "PlaylistHandlerTest.moc"
