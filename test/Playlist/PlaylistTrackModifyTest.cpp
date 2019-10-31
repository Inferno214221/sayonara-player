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
	MetaDataList v_md = Test::Playlist::create_v_md(0, 100);

	int i=0;
	for(auto it=v_md.begin(); it != v_md.end(); it++, i++)
	{
		it->set_artist("artist0");
	}

	auto pl = std::make_shared<PL>(1, PlaylistType::Std, "Hallo");
	pl->create_playlist(v_md);

	MetaDataList v_md_old, v_md_new;
	std::copy_n(v_md.begin(), 10, std::back_inserter(v_md_old));
	v_md_new = v_md_old;

	i = 0;
	for(auto it=v_md_new.begin(); it != v_md_new.end(); it++, i++)
	{
		it->set_artist( QString("artist%1").arg(i) );
	}

	auto* mdcn = Tagging::ChangeNotifier::instance();
	QSignalSpy spy(mdcn, &Tagging::ChangeNotifier::sig_metadata_changed);

	mdcn->change_metadata(v_md_old, v_md_new);

	QCOMPARE(spy.count(), 1);

	v_md = pl->tracks();
	i = 0;
	for(auto it=v_md.begin(); it != v_md.end(); it++, i++)
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
	MetaDataList v_md = Test::Playlist::create_v_md(0, 100);

	int i=0;
	for(auto it=v_md.begin(); it != v_md.end(); it++, i++)
	{
		it->set_artist("artist0");
	}

	auto pl = std::make_shared<PL>(1, PlaylistType::Std, "Hallo");
	pl->create_playlist(v_md);

	MetaDataList v_md_to_delete;
	std::copy_if(v_md.begin(), v_md.end(), std::back_inserter(v_md_to_delete), [](const MetaData& md){
		return (md.id() % 5 == 0);
	});

	auto* mdcn = Tagging::ChangeNotifier::instance();
	QSignalSpy spy(mdcn, &Tagging::ChangeNotifier::sig_metadata_deleted);

	mdcn->delete_metadata(v_md_to_delete);

	QCOMPARE(spy.count(), 1);

	v_md = pl->tracks();
	i = 0;
	for(auto it=v_md.begin(); it != v_md.end(); it++, i++)
	{
		QVERIFY((it->id() % 5) != 0);
	}
}

QTEST_GUILESS_MAIN(PlaylistTrackModifyTest)

#include "PlaylistTrackModifyTest.moc"

