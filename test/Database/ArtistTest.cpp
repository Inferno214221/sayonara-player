#include "test/Common/SayonaraTest.h"
#include "test/Common/DatabaseUtils.h"

#include "Database/Connector.h"
#include "Database/Artists.h"
#include "Database/LibraryDatabase.h"
#include "Database/Library.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Algorithm.h"

namespace
{
	constexpr const LibraryId testLibraryId = 0;

	struct MetaDataBlock
	{
		QString album;
		QString artist;
		QString albumArtist;
		QString title;
	};

	[[maybe_unused]] MetaData createTestTrack(const MetaDataBlock& data)
	{
		auto track = MetaData {};
		track.setTitle(data.title);
		track.setAlbum(data.album);
		track.setArtist(data.artist);
		track.setAlbumArtist(data.albumArtist);

		const auto path = QString("/path/to/%1/%2/%3.mp3")
			.arg(data.artist)
			.arg(data.album)
			.arg(data.title);
		track.setFilepath(path);

		return track;
	}

	void cleanLibraryDatabase(DB::LibraryDatabase* db)
	{
		auto tracks = MetaDataList {};
		db->getAllTracks(tracks);
		auto ids = IdList {};
		Util::Algorithm::transform(tracks, ids, [](const auto& track) {
			return track.id();
		});

		Test::DB::clearDatabase(db);
	}

	void createLibraryDatabase()
	{
		auto* dbConnector = DB::Connector::instance();
		dbConnector->deleteLibraryDatabase(testLibraryId);
		dbConnector->registerLibraryDatabase(testLibraryId);

		auto* db = dbConnector->libraryDatabase(testLibraryId, 0);
		cleanLibraryDatabase(db);
	}

	void createTestLibrary(const QList<MetaDataBlock>& data)
	{
		createLibraryDatabase();
		auto* db = DB::Connector::instance()->libraryDatabase(testLibraryId, 0);

		auto tracks = MetaDataList {};
		Util::Algorithm::transform(data, tracks, [&](const auto& dataItem) {
			return createTestTrack(dataItem);
		});

		db->storeMetadata(tracks);

		QVERIFY(db->getNumTracks() == data.count());
	}
}

class ArtistTest :
	public Test::Base
{
	Q_OBJECT

	public:
		ArtistTest() :
			Test::Base("ArtistTest") {}

		~ArtistTest() override = default;

	private slots:
		[[maybe_unused]] void testFetch();
		[[maybe_unused]] void testDelete();
};

[[maybe_unused]] void ArtistTest::testFetch() // NOLINT(readability-convert-member-functions-to-static)
{
	constexpr const auto testCases = std::array {
		std::tuple {DB::ArtistIdInfo::ArtistIdField::ArtistId, 6, 3},
		std::tuple {DB::ArtistIdInfo::ArtistIdField::AlbumArtistId, 6, 2}
	};

	for(const auto& [artistIdField, expectedWithEmpty, expectedWithoutEmpty]: testCases)
	{
		createTestLibrary({
			                  {"album1", "artist1",            "albumArtist1", "title1"},
			                  {"album2", "artist1 feat. Dude", "albumArtist1", "title2"},
			                  {"album3", "artist3",            "albumArtist3", "title3"}
		                  });

		auto* db = DB::Connector::instance()->libraryDatabase(testLibraryId, 0);
		db->changeArtistIdField(artistIdField);
		db->insertArtistIntoDatabase("Orphan");

		auto artists = ArtistList {};
		db->getAllArtists(artists, true);
		QVERIFY(artists.count() == expectedWithEmpty);

		db->getAllArtists(artists, false);
		QVERIFY(artists.count() == expectedWithoutEmpty);
	}
}

[[maybe_unused]] void ArtistTest::testDelete() // NOLINT(readability-convert-member-functions-to-static)
{
	createTestLibrary({
		                  {"album1", "artist1", "albumArtist1", "title1"},
		                  {"album2", "artist2", "albumArtist2", "title2"},
		                  {"album3", "artist3", "albumArtist3", "title3"}
	                  });

	auto* db = DB::Connector::instance()->libraryDatabase(testLibraryId, 0);

	auto artists = ArtistList {};
	db->getAllArtists(artists, true);
	QVERIFY(artists.count() == 6);

	db->deleteArtist(db->getArtistID("artist1"));
	db->getAllArtists(artists, true);
	QVERIFY(artists.count() == 5);

	db->deleteArtist(db->getArtistID("artist1"));
	db->getAllArtists(artists, true);
	QVERIFY(artists.count() == 5);

	db->deleteArtist(db->getArtistID("albumArtist2"));
	db->getAllArtists(artists, true);
	QVERIFY(artists.count() == 4);

	db->deleteArtist(db->getArtistID("albumArtist3"));
	db->getAllArtists(artists, true);
	QVERIFY(artists.count() == 3);
}

QTEST_GUILESS_MAIN(ArtistTest)

#include "ArtistTest.moc"
