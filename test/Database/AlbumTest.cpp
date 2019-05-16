#include <QTest>
#include <QObject>
#include <QMap>

#include "Database/Connector.h"
#include "Database/Albums.h"
#include "Database/LibraryDatabase.h"
#include "Database/Library.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Utils.h"

class AlbumTest : public QObject
{
	Q_OBJECT

private:
	QStringList m_album_names;

public:
	AlbumTest() : QObject()
	{
		QFile::remove("/tmp/player.db");
		for(int i=0; i<100; i++)
		{
			m_album_names << Util::random_string(Util::random_number(5, 20));
		}
	}

	~AlbumTest()
	{
		QFile::remove("/tmp/player.db");
	}

private:
	DB::LibraryDatabase* album_connector()
	{
		DB::Connector* db = DB::Connector::instance("/tmp", "player.db");
		db->register_library_db(0);
		return db->library_db(0, 0);
	}

private slots:
	void test_insert();
	void test_insert_known();
	void test_rename();
};


void AlbumTest::test_insert()
{
	for(const QString& album_name : m_album_names)
	{
		album_connector()->insertAlbumIntoDatabase(album_name);
	}

	bool success;
	AlbumList albums;
	success = album_connector()->getAllAlbums(albums, true);

	QVERIFY(success == true);
	QVERIFY(albums.count() == m_album_names.size());
}

void AlbumTest::test_insert_known()
{
	AlbumId id = album_connector()->insertAlbumIntoDatabase(m_album_names.first());
	QVERIFY(id == -1);

	AlbumList albums;
	bool success = album_connector()->getAllAlbums(albums, true);

	QVERIFY(success == true);
	QVERIFY(albums.count() == m_album_names.size());
}

void AlbumTest::test_rename()
{
	Album album;

	{ // fetch random album from db
		AlbumId id = album_connector()->getAlbumID(m_album_names[1]);
		QVERIFY(id >= 0);

		bool success = album_connector()->getAlbumByID(id, album, true);
		QVERIFY(success == true);
		QVERIFY(album.id == id);
		QVERIFY(album.name() == m_album_names[1]);
	}

	{ // rename album with new name
		QString new_name = Util::random_string(32);
		album.set_name(new_name);
		AlbumId id = album_connector()->updateAlbum(album);
		QVERIFY(id == album.id);
	}

	{ // rename album with already existing name
		QString new_name = m_album_names[2];
		album.set_name(new_name);
		AlbumId id = album_connector()->updateAlbum(album);
		QVERIFY(id == -1);
	}

	{ // check albums
		AlbumList albums;
		bool success = album_connector()->getAllAlbums(albums, true);

		QVERIFY(success == true);
		QVERIFY(albums.count() == m_album_names.size());
	}
}

QTEST_MAIN(AlbumTest)

#include "AlbumTest.moc"
