/* LibraryDatabaseTest.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test/Common/SayonaraTest.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Database/Library.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataSorting.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

#include <QMap>

class LibraryDatabaseTest :
	public Test::Base
{
	Q_OBJECT

	private:
		QStringList mAlbumNames;
		DB::LibraryDatabase* mLibraryDatabase = nullptr;

	public:
		LibraryDatabaseTest() :
			Test::Base("LibraryDatabaseTest") {}

		~LibraryDatabaseTest() override = default;

	private:
		DB::LibraryDatabase* initDatabase();

	private slots:
		void testStore();
};

static MetaDataList createTracks()
{
	MetaDataList tracks;
	for(int i = 0; i < 100; i++)
	{
		MetaData md;

		md.setFilepath(QString("/path/to/file%1.mp3").arg(i));
		md.setArtist(QString("Artist%1").arg(i / 20));
		md.setAlbum(QString("Album%1").arg(i / 10));
		md.setTitle(QString("Title%1").arg(i));
		md.setTrackNumber(i % 10);
		md.setYear(2000 + i / 20);
		md.setBitrate(Util::randomNumber(0, 320000));

		tracks << md;
	}

	// track[00-09]: artist0, album0
	// track[10-19]: artist0, album1
	// track[20-29]: artist1, album2
	// track[30-39]: artist1, album3
	// track[40-49]: artist2, album4
	// track[50-59]: artist2, album5
	// track[60-69]: artist3, album6
	// track[70-79]: artist3, album7
	// track[80-89]: artist4, album8
	// track[90-99]: artist4, album9

	MetaDataSorting::sortMetadata(tracks, Library::TrackSortorder::TitleAsc);
	return tracks;
}

void LibraryDatabaseTest::testStore()
{
	const MetaDataList tracks = createTracks();
	QVERIFY(tracks.count() == 100);

	auto* libraryDb = initDatabase();
	bool success = libraryDb->storeMetadata(tracks);

	QVERIFY(success == true);

	int trackcount = libraryDb->getNumTracks();
	qDebug() << "trackcount " << trackcount;
	QVERIFY(trackcount == 100);

	MetaDataList tempTracks;
	success = libraryDb->getAllTracks(tempTracks);
	QVERIFY(success == true);
	QVERIFY(tempTracks.count() == tracks.count());

	MetaDataSorting::sortMetadata(tempTracks, Library::TrackSortorder::TitleAsc);
	for(int i = 0; i < tracks.count(); i++)
	{
		QVERIFY(tracks[i] == tempTracks[i]);
	}

	AlbumList albums;
	libraryDb->getAllAlbums(albums, true);
	QVERIFY(albums.count() == 10);

	ArtistList artists;
	libraryDb->getAllArtists(artists, true);
	QVERIFY(artists.count() == 5);

	MetaData md1 = tracks[0];
	md1.setArtist("Some new Artist");

	MetaData md2 = tracks[0];
	md2.setArtist("");
	md2.setAlbum("");
	md2.setFilepath("/a/completely/different/path.mp3");

	tempTracks.clear();
	tempTracks << md1 << md2;

	QVERIFY(tempTracks.count() == 2);
	success = libraryDb->storeMetadata(tempTracks);
	QVERIFY(success == true);

	trackcount = libraryDb->getNumTracks();
	QVERIFY(trackcount == tracks.count() + 1);

	success = libraryDb->getAllArtists(artists, true);
	QVERIFY(success);
	QVERIFY(artists.count() == 7);

	success = libraryDb->getAllAlbums(albums, true);
	QVERIFY(success);
	QVERIFY(albums.count() == 12);

	success = libraryDb->getAllTracks(tempTracks);
	qDebug() << "New trackcount " << tempTracks.count();
	QVERIFY(success);
	QVERIFY(tempTracks.count() == trackcount);

	libraryDb->storeMetadata(tracks);
	trackcount = libraryDb->getNumTracks();
	QVERIFY(trackcount == tracks.count() + 1);

	libraryDb->getAllArtists(artists, true);
	QVERIFY(artists.count() == 7);

	libraryDb->getAllAlbums(albums, true);
	QVERIFY(albums.count() == 12);

	// md1 is overwritten again -> artist of md1 has no tracks anymore
	// md2 stays as the filepath differs -> artist of md2 stays
	libraryDb->getAllArtists(artists, false);
	QVERIFY(artists.count() == 6);

	// album of md2 stays
	libraryDb->getAllAlbums(albums, false);
	QVERIFY(albums.count() == 11);
}

DB::LibraryDatabase* LibraryDatabaseTest::initDatabase()
{
	if(mLibraryDatabase)
	{
		return mLibraryDatabase;
	}

	auto* db = DB::Connector::instance();
	db->registerLibraryDatabase(0);
	mLibraryDatabase = db->libraryDatabase(0, 0);

	return mLibraryDatabase;
}

QTEST_GUILESS_MAIN(LibraryDatabaseTest)

#include "LibraryDatabaseTest.moc"
