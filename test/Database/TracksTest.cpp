/* TestDatabase.cpp */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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
#include "test/Common/TestTracks.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Library/Filter.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"

namespace
{
	constexpr const auto SearchMode = (+Library::SearchMode::CaseInsensitve | +Library::SearchMode::NoSpecialChars);

	bool findTrack(const MetaData& track, const MetaDataList& tracks)
	{
		return Util::Algorithm::contains(tracks, [&](const auto& element) {
			return (element.filepath() == track.filepath());
		});
	}

	bool isSubset(const MetaDataList& needle, const MetaDataList& hayStack)
	{
		return std::all_of(std::begin(needle), std::end(needle), [&](const auto& track) {
			return findTrack(track, hayStack);
		});
	}

	Library::Filter createFulltextFilter(const QString& str)
	{
		Library::Filter filter;
		filter.setFiltertext(str);
		filter.setMode(Library::Filter::Mode::Fulltext);

		return filter;
	}
}

class TracksTest :
	public Test::Base
{
	Q_OBJECT

	public:
		TracksTest() :
			Test::Base("TracksTest")
		{
			m_testTracks = Test::createTracks();
			Settings::instance()->set<Set::Lib_SearchMode>(SearchMode);
		}

	private slots:
		[[maybe_unused]] void testGetAllTracks();
		[[maybe_unused]] void testFetchByAlbumArtist();
		[[maybe_unused]] void testFetchByAlbumArtistWithFilter();
		[[maybe_unused]] void testFetchByArtist();
		[[maybe_unused]] void testFetchByArtistWithFilter();
		[[maybe_unused]] void testFetchByAlbum();
		[[maybe_unused]] void testFetchByAlbumWithFilter();
		[[maybe_unused]] void testSearchByFulltext();
		[[maybe_unused]] void testSearchByGenre();
		[[maybe_unused]] void testSearchByFilepath();
		[[maybe_unused]] void testGetAllGenres();
		[[maybe_unused]] void testGetByPaths();
		[[maybe_unused]] void testInsertAndUpdate();
		[[maybe_unused]] void testInsertAndDelete();
		[[maybe_unused]] void testRenameFilepath();

	private: // NOLINT(readability-redundant-access-specifiers)
		DB::LibraryDatabase*
		initDatabase(DB::LibraryDatabase::ArtistIDField artistIdField = DB::LibraryDatabase::ArtistIDField::ArtistID)
		{
			if(m_libraryDatabase)
			{
				m_libraryDatabase->changeArtistIdField(artistIdField);
				return m_libraryDatabase;
			}

			auto* db = DB::Connector::instance();
			db->registerLibraryDatabase(0);
			m_libraryDatabase = db->libraryDatabase(0, 0);
			m_libraryDatabase->storeMetadata(m_testTracks);
			m_libraryDatabase->updateSearchMode();
			m_libraryDatabase->changeArtistIdField(artistIdField);

			return m_libraryDatabase;
		}

		DB::LibraryDatabase* m_libraryDatabase = nullptr;
		MetaDataList m_testTracks;
};

[[maybe_unused]] void TracksTest::testGetAllTracks()
{
	auto* db = initDatabase();
	MetaDataList tracks;
	db->getAllTracks(tracks);

	QVERIFY(tracks.size() == m_testTracks.size());
	QVERIFY(isSubset(tracks, m_testTracks));
}

[[maybe_unused]] void TracksTest::testFetchByAlbumArtist()
{
	auto* db = initDatabase(DB::LibraryDatabase::ArtistIDField::AlbumArtistID);

	MetaDataList tracks;
	const auto artistId = db->getArtistID("Europe");
	db->getAllTracksByArtist({artistId}, tracks);

	QVERIFY(tracks.size() == 4);
	QVERIFY(isSubset(tracks, m_testTracks));
}

[[maybe_unused]] void TracksTest::testFetchByAlbumArtistWithFilter()
{
	auto* db = initDatabase(DB::LibraryDatabase::ArtistIDField::AlbumArtistID);

	MetaDataList tracks;
	const auto artistId = db->getArtistID("Africa");
	const auto filter = createFulltextFilter("earth");

	db->getAllTracksByArtist({artistId}, tracks, filter);

	QVERIFY(tracks.size() == 5);
	QVERIFY(isSubset(tracks, m_testTracks));
}

[[maybe_unused]] void TracksTest::testFetchByArtist()
{
	auto* db = initDatabase();

	MetaDataList tracks;
	const auto artistId = db->getArtistID("Earth feat. Europe");
	db->getAllTracksByArtist({artistId}, tracks);

	QVERIFY(tracks.size() == 4);
	QVERIFY(isSubset(tracks, m_testTracks));
}

[[maybe_unused]] void TracksTest::testFetchByArtistWithFilter()
{
	auto* db = initDatabase();

	MetaDataList tracks;
	const auto artistId = db->getArtistID("Earth feat. Europe");
	const auto filter = createFulltextFilter("earth");

	db->getAllTracksByArtist({artistId}, tracks, filter);

	QVERIFY(tracks.size() == 4);
	QVERIFY(isSubset(tracks, m_testTracks));
}

[[maybe_unused]] void TracksTest::testFetchByAlbum()
{
	auto* db = initDatabase();

	MetaDataList tracks;
	const auto albumId = db->getAlbumID("Europe Mountains");
	db->getAllTracksByAlbum({albumId}, tracks);

	QVERIFY(tracks.size() == 4);
	QVERIFY(isSubset(tracks, m_testTracks));
}

[[maybe_unused]] void TracksTest::testFetchByAlbumWithFilter()
{
	auto* db = initDatabase();

	MetaDataList tracks;
	const auto albumId = db->getAlbumID("Europe Mountains");
	const auto filter = createFulltextFilter("earth");

	db->getAllTracksByAlbum({albumId}, tracks, filter, -1);

	QVERIFY(tracks.size() == 4);
	QVERIFY(isSubset(tracks, m_testTracks));
}

[[maybe_unused]] void TracksTest::testSearchByFulltext()
{
	{ // by album "African Countries"
		auto* db = initDatabase();
		const auto filter = createFulltextFilter("africancountri");

		MetaDataList tracks;
		db->getAllTracksBySearchString(filter, tracks);

		QVERIFY(tracks.size() == 5);
		QVERIFY(isSubset(tracks, m_testTracks));
	}

	{ // by artist "earth feat. *"
		auto* db = initDatabase();
		const auto filter = createFulltextFilter("earthfeat");

		MetaDataList tracks;
		db->getAllTracksBySearchString(filter, tracks);

		QVERIFY(tracks.size() == 12);
		QVERIFY(isSubset(tracks, m_testTracks));
	}

	{ // by title "Matterhorn"
		auto* db = initDatabase();
		const auto filter = createFulltextFilter("atterhor");

		MetaDataList tracks;
		db->getAllTracksBySearchString(filter, tracks);

		QVERIFY(tracks.size() == 1);
		QVERIFY(isSubset(tracks, m_testTracks));
	}
}

[[maybe_unused]] void TracksTest::testSearchByGenre()
{
	Library::Filter filter;
	filter.setFiltertext("asiarivers");
	filter.setMode(Library::Filter::Mode::Genre);

	auto* db = initDatabase();

	MetaDataList tracks;
	db->getAllTracksBySearchString(filter, tracks);

	QVERIFY(tracks.size() == 3);
	QVERIFY(isSubset(tracks, m_testTracks));
}

[[maybe_unused]] void TracksTest::testSearchByFilepath()
{
	Library::Filter filter;
	filter.setFiltertext("rivers");
	filter.setMode(Library::Filter::Mode::Filename);

	auto* db = initDatabase();

	MetaDataList tracks;
	db->getAllTracksBySearchString(filter, tracks);

	QVERIFY(tracks.size() == 3);
	QVERIFY(isSubset(tracks, m_testTracks));
}

[[maybe_unused]] void TracksTest::testGetAllGenres()
{
	auto* db = initDatabase();

	ArtistList artists;
	AlbumList albums;
	db->getAllArtists(artists, false);
	db->getAllAlbums(albums, false);

	const auto genres = db->getAllGenres();
	QVERIFY(genres.size() == (artists.size() + albums.size()));
}

[[maybe_unused]] void TracksTest::testGetByPaths()
{
	QStringList paths;
	for(int i = 0; i < 7; i++) // NOLINT(readability-magic-numbers)
	{
		paths << m_testTracks[i].filepath();
	}

	QStringList incompletePaths;
	for(int i = 8; i < 10; i++) // NOLINT(readability-magic-numbers)
	{
		const auto path = m_testTracks[i].filepath();
		incompletePaths << path.left(path.size() - 3);
	}

	paths.append(incompletePaths);

	auto* db = initDatabase();
	{
		MetaDataList tracks;
		db->getMultipleTracksByPath(paths, tracks);

		QVERIFY(tracks.count() == (paths.count() - incompletePaths.size()));
	}

	{
		MetaDataList tracks;
		db->getAllTracksByPaths(paths, tracks);

		QVERIFY(tracks.count() == paths.count());
	}
}

[[maybe_unused]] void TracksTest::testInsertAndUpdate() // NOLINT(readability-function-cognitive-complexity)
{
	auto* db = initDatabase();

	const auto originalTrack = Test::createTrack(1, "title", "artist", "album", "albumArtist");
	const auto path = originalTrack.filepath();
	const auto success = db->storeMetadata(MetaDataList {originalTrack});
	QVERIFY(success);

	{
		auto track = db->getTrackByPath(path);
		QVERIFY(track.id() >= 0);
		QVERIFY(track.title() == "title");
		QVERIFY(track.artist() == "artist");
		QVERIFY(track.album() == "album");
		QVERIFY(track.albumArtist() == "albumArtist");

		track.setBitrate(128'000); // NOLINT(readability-magic-numbers)
		track.setComment("newComment");
		track.setDiscnumber(5); // NOLINT(readability-magic-numbers)
		track.setFilesize(200'000); // NOLINT(readability-magic-numbers)
		track.setGenres(QStringList() << "newGenre1" << "newGenre2");
		track.setDurationMs(200'000); // NOLINT(readability-magic-numbers)
		track.setRating(Rating::One);
		track.setTitle("newTitle");
		track.setTrackNumber(2);
		track.setYear(1994); // NOLINT(readability-magic-numbers)

		QVERIFY(db->updateTrack(track));
	}

	{
		const auto track = db->getTrackByPath(path);
		QVERIFY(track.id() >= 0);
		QVERIFY(track.bitrate() == 128'000);
		QVERIFY(track.comment() == "newComment");
		QVERIFY(track.discnumber() == 5);
		QVERIFY(track.filesize() == 200'000);
		QVERIFY(track.genres().size() == 2);
		QVERIFY(track.genres().contains(Genre("newGenre1")));
		QVERIFY(track.genres().contains(Genre("newGenre2")));
		QVERIFY(track.durationMs() == 200'000);
		QVERIFY(track.rating() == Rating::One);
		QVERIFY(track.title() == "newTitle");
		QVERIFY(track.trackNumber() == 2);
		QVERIFY(track.year() == 1994);
	}
}

[[maybe_unused]] void TracksTest::testInsertAndDelete()
{
	auto* db = initDatabase();

	const auto originalTrack =
		Test::createTrack(1, "titleToDelete", "artistToDelete", "albumToDelete", "albumArtistToDelete");
	const auto path = originalTrack.filepath();
	const auto success = db->storeMetadata(MetaDataList {originalTrack});
	QVERIFY(success);

	{
		const auto track = db->getTrackByPath(path);
		QVERIFY(track.id() >= 0);
		QVERIFY(!track.isEqualDeep(MetaData {}));
		db->deleteTrack(track.id());
	}

	{
		const auto deletedTrack = db->getTrackByPath(path);
		QVERIFY(deletedTrack.id() < 0);
		QVERIFY(deletedTrack.isEqualDeep(MetaData {}));
	}
}

[[maybe_unused]] void TracksTest::testRenameFilepath()
{
	auto* db = initDatabase();

	const auto originalTrack =
		Test::createTrack(1, "titleToRename", "artistToRename", "albumToRename", "albumArtistToRename");
	const auto path = originalTrack.filepath();
	const auto newPath = QStringLiteral("/some/new/path.mp3");
	const auto success = db->storeMetadata(MetaDataList {originalTrack});
	QVERIFY(success);

	{
		const auto track = db->getTrackByPath(path);
		QVERIFY(track.id() >= 0);
		db->renameFilepath(path, newPath, track.libraryId());
	}

	{
		const auto nonExistentTrack = db->getTrackByPath(path);
		QVERIFY(nonExistentTrack.isEqualDeep(MetaData {}));

		const auto trackWithNewPath = db->getTrackByPath(newPath);
		QVERIFY(trackWithNewPath.id() >= 0);
		QVERIFY(trackWithNewPath.filepath() == newPath);
	}
}

QTEST_GUILESS_MAIN(TracksTest)

#include "TracksTest.moc"
