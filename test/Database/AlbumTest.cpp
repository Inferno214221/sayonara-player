#include "SayonaraTest.h"

#include "Database/Connector.h"
#include "Database/Albums.h"
#include "Database/LibraryDatabase.h"
#include "Database/Library.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

#include <QMap>

class AlbumTest :
	public SayonaraTest
{
	Q_OBJECT

private:
	QStringList m_album_names;
	DB::LibraryDatabase* m_lib_db=nullptr;

public:
	AlbumTest() :
		SayonaraTest("AlbumTest")
	{}

	~AlbumTest() override = default;

private:
	DB::LibraryDatabase* init();

private slots:
	void test_insert();
	void test_insert_known();
	void test_rename();
};

DB::LibraryDatabase* AlbumTest::init()
{
	if(m_lib_db){
		return m_lib_db;
	}

	m_album_names.clear();

	for(int i=0; i<100; i++)
	{
		m_album_names << Util::random_string(Util::random_number(5, 20));
	}

	auto* db = DB::Connector::instance();
	db->register_library_db(0);
	m_lib_db = db->library_db(0, 0);

	for(const QString& album_name : m_album_names)
	{
		m_lib_db->insertAlbumIntoDatabase(album_name);
	}

	return m_lib_db;
}

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

QTEST_GUILESS_MAIN(AlbumTest)

#include "AlbumTest.moc"
