#include "SayonaraTest.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Database/Library.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

#include <QMap>

class LibraryDatabaseTest :
	public Test::Base
{
	Q_OBJECT

private:
	QStringList m_album_names;
	DB::LibraryDatabase* m_lib_db=nullptr;

public:
	LibraryDatabaseTest() :
		Test::Base("LibraryDatabaseTest")
	{}

	~LibraryDatabaseTest() override = default;

private:
	DB::LibraryDatabase* init_db();

private slots:
	void test_store();
};

static MetaDataList create_tracks()
{
	MetaDataList tracks;
	for(int i=0; i<100; i++)
	{
		MetaData md;

		md.setFilepath( QString("/path/to/file%1.mp3").arg(i) );
		md.setArtist( QString("Artist%1").arg(i / 20) );
		md.setAlbum( QString("Album%1").arg(i / 10) );
		md.setTitle( QString("Title%1").arg(i) );
		md.setTrackNumber(i % 10);
		md.setYear( 2000 + i / 20 );
		md.setBitrate(Util::randomNumber(0, 320000));

		tracks << md;
	}

	tracks.sort(Library::SortOrder::TrackTitleAsc);
	return tracks;
}

void LibraryDatabaseTest::test_store()
{
	const MetaDataList tracks = create_tracks();
	QVERIFY(tracks.count() == 100);

	auto* lib_db = init_db();
	bool success = lib_db->storeMetadata(tracks);

	QVERIFY(success == true);

	int trackcount = lib_db->getNumTracks();
	qDebug() << "trackcount " << trackcount;
	QVERIFY(trackcount == 100);

	MetaDataList tracks_tmp;
	success = lib_db->getAllTracks(tracks_tmp);
	QVERIFY(success == true);
	QVERIFY(tracks_tmp.count() == tracks.count());

	tracks_tmp.sort(Library::SortOrder::TrackTitleAsc);
	for(int i=0; i<tracks.count(); i++)
	{
		QVERIFY(tracks[i] == tracks_tmp[i]);
	}

	AlbumList albums;
	lib_db->getAllAlbums(albums, true);
	QVERIFY(albums.count() == 10);

	ArtistList artists;
	lib_db->getAllArtists(artists, true);
	QVERIFY(artists.count() == 5);

	MetaData md1 = tracks.first();
	md1.setArtist("Some new Artist");

	MetaData md2 = tracks.first();
	md2.setArtist("");
	md2.setAlbum("");
	md2.setFilepath("/a/completely/different/path.mp3");

	tracks_tmp.clear();
	tracks_tmp << md1 << md2;

	QVERIFY(tracks_tmp.count() == 2);
	success = lib_db->storeMetadata(tracks_tmp);
	QVERIFY(success == true);

	trackcount = lib_db->getNumTracks();
	QVERIFY(trackcount == tracks.count() + 1);

	success = lib_db->getAllArtists(artists, true);
	QVERIFY(success);
	QVERIFY(artists.count() == 7);

	success = lib_db->getAllAlbums(albums, true);
	QVERIFY(success);
	QVERIFY(albums.count() == 11);

	success = lib_db->getAllTracks(tracks_tmp);
	qDebug() << "New trackcount " << tracks_tmp.count();
	QVERIFY(success);
	QVERIFY(tracks_tmp.count() == trackcount);

	lib_db->storeMetadata(tracks);
	trackcount = lib_db->getNumTracks();
	QVERIFY(trackcount == tracks.count() + 1);

	lib_db->getAllArtists(artists, true);
	QVERIFY(artists.count() == 7);

	lib_db->getAllAlbums(albums, true);
	QVERIFY(albums.count() == 11);

	// md1 is overwritten again -> artist of md1 has no tracks anymore
	// md2 stays as the filepath differs -> artist of md2 stays
	lib_db->getAllArtists(artists, false);
	QVERIFY(artists.count() == 6);

	// album of md2 stays
	lib_db->getAllAlbums(albums, false);
	QVERIFY(albums.count() == 11);
}


DB::LibraryDatabase* LibraryDatabaseTest::init_db()
{
	if(m_lib_db)
	{
		return m_lib_db;
	}

	auto* db = DB::Connector::instance();
	db->registerLibraryDatabase(0);
	m_lib_db = db->libraryDatabase(0, 0);

	return m_lib_db;
}


QTEST_GUILESS_MAIN(LibraryDatabaseTest)

#include "LibraryDatabaseTest.moc"
