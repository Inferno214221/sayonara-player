#include "SayonaraTest.h"
#include "PlayManagerMock.h"
#include "Playlist/PlaylistTestUtils.h"

#include "Components/Playlist/Playlist.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QSignalSpy>

#include <iterator>

using PL=Playlist::Playlist;

class PlaylistTrackModifyTest :
	public Test::Base
{
	Q_OBJECT

public:
	PlaylistTrackModifyTest() :
		Test::Base("PlaylistTrackModifyTest"),
		m_playManager{new PlayManagerMock()}
	{}

	~PlaylistTrackModifyTest()
	{
		delete m_playManager;
	}

private:
	PlayManager* m_playManager;


private slots:
	void trackModifiedTest();
	void trackDeletedTest();
};

void PlaylistTrackModifyTest::trackModifiedTest()
{
	// create tracks and move them to a playlist
	// change the original tracks
	// fetch the tracks from the playlist again
	// and see if the have been modified correctly

	MetaDataList tracks = Test::Playlist::createTrackList(0, 100);

	// give all tracks the same artist
	for(auto it=tracks.begin(); it != tracks.end(); it++)
	{
		it->setArtist("artist0");
	}

	// create a playlist
	auto pl = std::make_shared<PL>(1, "Hallo", m_playManager);
	pl->createPlaylist(tracks);

	MetaDataList oldTracks, newTracks;
	{ // give all tracks a new artist, except the first one
		std::copy_n(tracks.begin(), 10, std::back_inserter(oldTracks));
		newTracks = oldTracks;

		QVERIFY(newTracks.size() == oldTracks.size());

		{
			int i = 0;
			for(auto it=newTracks.begin(); it != newTracks.end(); it++, i++)
			{
				it->setArtist( QString("artist%1").arg(i) );
			}
		}
	}

	{ // trigger the Tagging::ChangeNotifier
		auto* mdcn = Tagging::ChangeNotifier::instance();
		QSignalSpy spy(mdcn, &Tagging::ChangeNotifier::sigMetadataChanged);

		QList<MetaDataPair> changedTracks;
		for(int i=0; i<newTracks.count(); i++)
		{
			changedTracks << QPair<MetaData, MetaData>(oldTracks[i], newTracks[i]);
		}

		mdcn->changeMetadata(changedTracks);

		// check if signal was fired
		int c = spy.count();
		QCOMPARE(c, 1);
	}

	{ // fetch the tracks from the playlist and see if they have been modified
		tracks = pl->tracks();
		int i = 0;
		for(auto it=tracks.begin(); it != tracks.end(); it++, i++)
		{
			if(i < 10) {
				QString artist = it->artist();
				QVERIFY(artist == QString("artist%1").arg(i));
			}

			else {
				QVERIFY(it->artist() == QString("artist0"));
			}
		}
	}
}

void PlaylistTrackModifyTest::trackDeletedTest()
{
	MetaDataList tracks = Test::Playlist::createTrackList(0, 100);

	int i=0;
	for(auto it=tracks.begin(); it != tracks.end(); it++, i++)
	{
		it->setArtist("artist0");
	}

	auto pl = std::make_shared<PL>(1, "Hallo", m_playManager);
	pl->createPlaylist(tracks);

	MetaDataList tracksToDelete;
	std::copy_if(tracks.begin(), tracks.end(), std::back_inserter(tracksToDelete), [](const MetaData& md){
		return (md.id() % 5 == 0);
	});

	auto* mdcn = Tagging::ChangeNotifier::instance();
	QSignalSpy spy(mdcn, &Tagging::ChangeNotifier::sigMetadataDeleted);

	mdcn->deleteMetadata(tracksToDelete);

	QCOMPARE(spy.count(), 1);

	tracks = pl->tracks();
	i = 0;
	for(auto it=tracks.begin(); it != tracks.end(); it++, i++)
	{
		QVERIFY((it->id() % 5) != 0);
	}
}

QTEST_GUILESS_MAIN(PlaylistTrackModifyTest)

#include "PlaylistTrackModifyTest.moc"

