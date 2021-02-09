#include "SayonaraTest.h"
#include "PlayManagerMock.h"
#include "PlaylistMocks.h"

#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"

#include "Interfaces/PlayManager.h"

#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/MetaData.h"

#include <memory>

// access working directory with Test::Base::tempPath("somefile.txt");

class PlaylistHandlerTest : 
    public Test::Base
{
    Q_OBJECT

    public:
        PlaylistHandlerTest() :
            Test::Base("PlaylistHandlerTest"),
            m_playManager(new PlayManagerMock())
        {}

	private:
		PlayManager* m_playManager;

		std::shared_ptr<Playlist::Handler> createHandler()
		{
    	    return std::make_shared<Playlist::Handler>(m_playManager, std::make_shared<PlaylistLoaderMock>());
		}

    private slots:
        void createTest();
        void closeTest();
        void currentIndexTest();
        void activeIndexTest();
};

void PlaylistHandlerTest::createTest()
{
	auto plh = createHandler();
	QVERIFY(plh->count() == 1);
	QVERIFY(plh->activeIndex() == 0);

	auto index = plh->createEmptyPlaylist(false);
	QVERIFY(plh->playlist(index)->index() == index);
	QVERIFY(index == 1);
	QVERIFY(plh->count() == 2);

	index = plh->createEmptyPlaylist(false);
	QVERIFY(plh->playlist(index)->index() == index);
	QVERIFY(index == 2);
	QVERIFY(plh->count() == 3);

	index = plh->createEmptyPlaylist(true);
	QVERIFY(plh->playlist(index)->index() == index);
	QVERIFY(index == 2);
	QVERIFY(plh->count() == 3);

	plh->shutdown();
}

void PlaylistHandlerTest::closeTest()
{
	auto plh = createHandler();
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
}

void PlaylistHandlerTest::currentIndexTest()
{
	auto plh = createHandler();

	QVERIFY(plh->currentIndex() == 0); // one playlist
	plh->setCurrentIndex(20);
	QVERIFY(plh->currentIndex() == 0);

	plh->createEmptyPlaylist(false); // two playlists
	QVERIFY(plh->currentIndex() == 1);

	plh->createEmptyPlaylist(false); // three playlists
	QVERIFY(plh->currentIndex() == 2);

	plh->setCurrentIndex(5); // invalid current index
	QVERIFY(plh->currentIndex() == 2);

	plh->setCurrentIndex(0); // valid current index
	QVERIFY(plh->currentIndex() == 0);

	plh->setCurrentIndex(2);
	plh->closePlaylist(2); // delete current index pl
	QVERIFY(plh->currentIndex() == 1);

	plh->closePlaylist(0); // delete last playlist
	QVERIFY(plh->currentIndex() == 0);
}

void PlaylistHandlerTest::activeIndexTest()
{
	auto plh = createHandler();
	QVERIFY(plh->activeIndex() == 0); // one playlist

	plh->createEmptyPlaylist();
	QVERIFY(plh->activeIndex() == 1); // two playlist

	plh->createEmptyPlaylist();
	QVERIFY(plh->activeIndex() == 2); // three playlists

	plh->closePlaylist(0);
	QVERIFY(plh->activeIndex() == 1); // two playlists

	MetaDataList tracks;
	for(int i=0; i<10; i++)
	{
		// file must exist
		const auto filename = Test::Base::tempPath(QString("file%1.mp3").arg(i));
		Util::File::writeFile(QByteArray{}, filename);

		tracks << MetaData{filename};
	}

	auto index = plh->createPlaylist(tracks, "new-playlist"); // index = 2

	plh->setCurrentIndex(0);
	QVERIFY(plh->activeIndex() == 0);

	auto playlist = plh->playlist(index);
	auto success = playlist->changeTrack(4, 0);
	QVERIFY(success);

	playlist->play();
	QVERIFY(plh->currentIndex() == 0);
	QVERIFY(plh->activeIndex() == 2);

	playlist->stop();
	QVERIFY(plh->activeIndex() == plh->currentIndex());
}

QTEST_GUILESS_MAIN(PlaylistHandlerTest)
#include "PlaylistHandlerTest.moc"
