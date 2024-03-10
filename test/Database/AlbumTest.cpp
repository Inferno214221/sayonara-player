/* AlbumTest.cpp
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
#include "test/Common/DatabaseUtils.h"

#include "Database/Connector.h"
#include "Database/Albums.h"
#include "Database/LibraryDatabase.h"
#include "Database/Library.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Algorithm.h"

#include <QMap>
#include <mutex>

static QStringList getHashes(const AlbumList& albums)
{
	QStringList hashes;
	Util::Algorithm::transform(albums, hashes, [](const Album& album) {
		return album.name() + album.albumArtist();
	});

	hashes.sort();
	return hashes;
}

static std::mutex mtx;

class AlbumTest :
	public Test::Base
{
	Q_OBJECT

	private:
		QStringList mAlbumNames;
		DB::LibraryDatabase* mLibraryDatabase = nullptr;

	public:
		AlbumTest() :
			Test::Base("AlbumTest") {}

		~AlbumTest() override = default;

	private:
		DB::LibraryDatabase* init(int count);

	private slots:
		void testInsert();
		void testInsertWithAlbumArtists();
		void testInsertKnown();
};

DB::LibraryDatabase* AlbumTest::init(int count)
{
	mAlbumNames.clear();

	MetaDataList tracks;
	for(int i = 0; i < count; i++)
	{
		MetaData md;

		QString albumName = Util::randomString(Util::randomNumber(5, 20));
		md.setAlbum(albumName);

		mAlbumNames << albumName;

		tracks << md;
	}

	auto* db = DB::Connector::instance();
	db->registerLibraryDatabase(0);

	if(!mLibraryDatabase)
	{
		mLibraryDatabase = db->libraryDatabase(0, 0);
	}

	Test::DB::clearDatabase(db);
	mLibraryDatabase->insertMissingArtistsAndAlbums(tracks);

	return mLibraryDatabase;
}

void AlbumTest::testInsert()
{
	std::lock_guard<std::mutex> lock(mtx);

	const int Count = 100;
	DB::LibraryDatabase* db = init(Count);

	bool success;
	AlbumList albums;
	success = db->getAllAlbums(albums, true);

	QVERIFY(success == true);
	QVERIFY(albums.count() == mAlbumNames.size());
	QVERIFY(albums.count() == Count);
}

void AlbumTest::testInsertWithAlbumArtists()
{
	std::lock_guard<std::mutex> lock(mtx);

	DB::LibraryDatabase* db = init(0);

	{
		AlbumList albums;
		bool success = db->getAllAlbums(albums, true);
		int count = albums.count();

		QVERIFY(success == true);
		QVERIFY(count == 0);
	}

	MetaDataList tracks;
	for(int albumArtist = 0; albumArtist < 5; albumArtist++)
	{
		QString albumArtistName = QString("albumArtist%1").arg(albumArtist);

		for(int album = 0; album < 4; album++)
		{
			MetaData md;

			QString albumName = QString("album%1").arg(album);

			md.setTitle(Util::randomString(10));
			md.setAlbum(albumName);
			md.setAlbumArtist(albumArtistName);
			md.setLibraryid(0);
			md.setFilepath(QString("somepath%1").arg(Util::randomNumber(10, 100000)));

			tracks << md;
		}
	}

	db->storeMetadata(tracks);

	{
		AlbumList albums;
		bool success = db->getAllAlbums(albums, true);
		QStringList hashes = getHashes(albums);

		QVERIFY(success == true);
		QVERIFY(albums.count() == (4 * 5));
	}

	tracks.clear();
	// 5 known and one additional artist with two albums
	for(int albumArtist = 0; albumArtist < 6; albumArtist++)
	{
		QString albumArtistName = QString("albumArtist%1").arg(albumArtist);

		for(int album = 0; album < 2; album++)
		{
			MetaData md;

			QString albumName = QString("album%1").arg(album);

			md.setTitle(Util::randomString(11));
			md.setAlbum(albumName);
			md.setAlbumArtist(albumArtistName);
			md.setLibraryid(0);
			md.setFilepath(QString("somepathasdf%1").arg(Util::randomNumber(10, 100000)));

			tracks << md;
		}
	}

	db->storeMetadata(tracks);

	{
		AlbumList albums;
		bool success = db->getAllAlbums(albums, true);
		QStringList hashes = getHashes(albums);

		QVERIFY(success == true);
		int albumCount = albums.count();
		QVERIFY(albumCount == (4 * 5) + 2);
	}
}

void AlbumTest::testInsertKnown()
{
	std::lock_guard<std::mutex> lock(mtx);

	const int Count = 100;
	DB::LibraryDatabase* db = init(Count);

	AlbumList albums;
	bool success;

	success = db->getAllAlbums(albums, true);
	QVERIFY(success == true);
	QVERIFY(albums.count() == mAlbumNames.size());

	MetaDataList tracks;
	for(int i = 0; i < Count / 2; i++)
	{
		int index = Util::randomNumber(0, mAlbumNames.size() - 1);
		MetaData md;
		md.setAlbum(mAlbumNames[index]);
		tracks << md;
	}

	MetaDataList modifiedTracks = db->insertMissingArtistsAndAlbums(tracks);
	for(const MetaData& md: modifiedTracks)
	{
		QVERIFY(md.albumId() >= 0);
	}

	albums.clear();
	success = db->getAllAlbums(albums, true);
	QVERIFY(success == true);
	QVERIFY(albums.count() == mAlbumNames.size());
	QVERIFY(albums.count() == Count);
}

QTEST_GUILESS_MAIN(AlbumTest)

#include "AlbumTest.moc"
