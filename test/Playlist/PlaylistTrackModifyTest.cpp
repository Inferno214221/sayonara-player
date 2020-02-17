#include "SayonaraTest.h"
#include "Playlist/PlaylistTestUtils.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Tagging/ChangeNotifier.h"

#include <QSignalSpy>

#include <iterator>

using PL=Playlist::Playlist;

class PlaylistTrackModifyTest :
	public Test::Base
{
	Q_OBJECT

public:
	PlaylistTrackModifyTest() :
		Test::Base("PlaylistTrackModifyTest")
	{}

private slots:
	void trackModifiedTest();
	void trackDeletedTest();
};


void PlaylistTrackModifyTest::trackModifiedTest()
{
	MetaDataList tracks = Test::Playlist::create_v_md(0, 100);

	int i=0;
	for(auto it=tracks.begin(); it != tracks.end(); it++, i++)
	{
		it->setArtist("artist0");
	}

	auto pl = std::make_shared<PL>(1, PlaylistType::Std, "Hallo");
	pl->createPlaylist(tracks);

	MetaDataList oldTracks, newTracks;
	std::copy_n(tracks.begin(), 10, std::back_inserter(oldTracks));
	newTracks = oldTracks;

	i = 0;
	for(auto it=newTracks.begin(); it != newTracks.end(); it++, i++)
	{
		it->setArtist( QString("artist%1").arg(i) );
	}

	auto* mdcn = Tagging::ChangeNotifier::instance();
	QSignalSpy spy(mdcn, &Tagging::ChangeNotifier::sigMetadataChanged);

	mdcn->changeMetadata(oldTracks, newTracks);

	QCOMPARE(spy.count(), 1);

	tracks = pl->tracks();
	i = 0;
	for(auto it=tracks.begin(); it != tracks.end(); it++, i++)
	{
		if(i < 10){
			QString artist = it->artist();
			QVERIFY(artist == QString("artist%1").arg(i));
		}

		else{
			QVERIFY(it->artist() == QString("artist0"));
		}
	}
}

void PlaylistTrackModifyTest::trackDeletedTest()
{
	MetaDataList tracks = Test::Playlist::create_v_md(0, 100);

	int i=0;
	for(auto it=tracks.begin(); it != tracks.end(); it++, i++)
	{
		it->setArtist("artist0");
	}

	auto pl = std::make_shared<PL>(1, PlaylistType::Std, "Hallo");
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

