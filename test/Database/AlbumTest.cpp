#include <QTest>
#include <QObject>
#include <QMap>

#include "Database/Connector.h"
#include "Database/Albums.h"
#include "Database/LibraryDatabase.h"
#include "Database/Library.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

class AlbumTest : public QObject
{
	Q_OBJECT

private:
	QStringList m_album_names;
	DB::LibraryDatabase* m_lib_db=nullptr;

public:
	AlbumTest()
	{
		Util::File::create_directories("/tmp/sayonara");
		//init();
	}
	~AlbumTest()
	{
		//QFile::remove("/tmp/sayonara/player.db");
	}

private:
	DB::LibraryDatabase* init()
	{
		if(m_lib_db){
			return m_lib_db;
		}

		m_album_names.clear();

		QFile::remove("/tmp/sayonara/player.db");
		for(int i=0; i<100; i++)
		{
			m_album_names << Util::random_string(Util::random_number(5, 20));
		}

		DB::Connector* db = DB::Connector::instance_custom(".", "/tmp/sayonara", "player.db");
		db->register_library_db(0);
		m_lib_db = db->library_db(0, 0);

		for(const QString& album_name : m_album_names)
		{
			m_lib_db->insertAlbumIntoDatabase(album_name);
		}

		return m_lib_db;
	}

private slots:
	void test_insert();
	void test_insert_known();
	void test_rename();
};


void AlbumTest::test_insert()
{
	DB::LibraryDatabase* db = init();

	bool success;
	AlbumList albums;
	success = db->getAllAlbums(albums, true);

	QVERIFY(success == true);
	QVERIFY(albums.count() == m_album_names.size());
	QVERIFY(albums.count() == 100);
}

void AlbumTest::test_insert_known()
{
	DB::LibraryDatabase* db = init();

	AlbumList albums;
	bool success;

	success = db->getAllAlbums(albums, true);
	QVERIFY(success == true);
	QVERIFY(albums.count() == m_album_names.size());

	AlbumId id = db->insertAlbumIntoDatabase(m_album_names.first());
	QVERIFY(id >= 0);

	albums.clear();
	success = db->getAllAlbums(albums, true);
	QVERIFY(success == true);
	QVERIFY(albums.count() == m_album_names.size());
}

void AlbumTest::test_rename()
{
	DB::LibraryDatabase* db = init();

	Album album;

	{ // fetch random album from db
		AlbumId id = db->getAlbumID(m_album_names[1]);
		QVERIFY(id >= 0);
		bool success = db->getAlbumByID(id, album, true);
		QVERIFY(success == true);
		QVERIFY(album.id == id);
		QVERIFY(album.name() == m_album_names[1]);
	}

	{ // check albums
		AlbumList albums;
		bool success = db->getAllAlbums(albums, true);

		QVERIFY(success == true);
		QVERIFY(albums.count() == m_album_names.size());
	}
}

QTEST_MAIN(AlbumTest)

#include "AlbumTest.moc"
