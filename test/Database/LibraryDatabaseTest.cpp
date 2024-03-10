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
#include "Database/Library.h"
#include "Database/LibraryDatabase.h"
#include "Database/Query.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"

#include <QMap>

namespace
{
	constexpr const auto libraryId = 0;
	constexpr const auto databaseId = 0;
	struct TestEnv
	{
		TestEnv()
		{
			cleanup();

			DB::Connector::instance()->registerLibraryDatabase(libraryId);

			// NOLINTNEXTLINE(*-prefer-member-initializer)
			libraryDatabase = DB::Connector::instance()->libraryDatabase(libraryId, databaseId);
		}

		~TestEnv()
		{
			cleanup();
		}

		void cleanup()
		{
			const auto q1 = DB::Connector::instance()->runQuery("DELETE FROM tracks;", "Cannot delete all tracks");
			QVERIFY(!DB::hasError(q1));

			const auto q2 = DB::Connector::instance()->runQuery("DELETE FROM albums;", "Cannot delete all albums");
			QVERIFY(!DB::hasError(q2));

			const auto q3 = DB::Connector::instance()->runQuery("DELETE FROM artists;", "Cannot delete all artists");
			QVERIFY(!DB::hasError(q3));

			const auto q4 = DB::Connector::instance()->runQuery("DELETE FROM Libraries;",
			                                                    "Cannot delete all libraries");
			QVERIFY(!DB::hasError(q4));
		}

		DB::LibraryDatabase* libraryDatabase {nullptr};
	};

	MetaData createTrack(const QString& artist, const QString& album, const QString& albumArtist, const QString& title)
	{
		const auto filepath = QString("/%1/%2/%3/%4%5.mp3")
			.arg(albumArtist)
			.arg(artist)
			.arg(album)
			.arg(title);

		auto track = MetaData {filepath};
		track.setArtist(artist);
		track.setAlbumArtist(albumArtist);
		track.setAlbum(album);
		track.setTitle(title);

		return track;
	}
}

class LibraryDatabaseTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LibraryDatabaseTest() :
			Test::Base("LibraryDatabaseTest") {}

		~LibraryDatabaseTest() override = default;

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testInsertTrackCreatesArtists()
		{
			[[maybe_unused]] auto env = TestEnv {};

			struct TestCase
			{
				MetaData track;
				QStringList expectedArtists;
			};

			const auto testCases = std::array {
				TestCase {createTrack("artist", "a", "albumArtist", "t1"), {"albumArtist", "artist"}},
				TestCase {createTrack("artist", "a", "albumArtist", "t2"), {"albumArtist", "artist"}},
				TestCase {createTrack("artist2", "a", "albumArtist", "t3"), {"albumArtist", "artist", "artist2"}},
				TestCase {createTrack("artist3", "a", "albumArtist", "t4"),
				          {"albumArtist", "artist", "artist2", "artist3"}}
			};

			for(const auto& testCase: testCases)
			{
				const auto tracks = MetaDataList() << testCase.track;
				env.libraryDatabase->storeMetadata(tracks);

				auto artists = ArtistList {};
				env.libraryDatabase->getAllArtists(artists, true);

				QCOMPARE(artists.count(), testCase.expectedArtists.count());
				for(const auto& name: testCase.expectedArtists)
				{
					const auto contains = Util::Algorithm::contains(artists, [&](const auto& artist) {
						return (artist.name() == name);
					});

					QVERIFY(contains);
				}
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testInsertTrackCreatesAlbums()
		{
			[[maybe_unused]] auto env = TestEnv {};

			struct TestCase
			{
				MetaData track;
				QList<QPair<QString, QString>> expectedAlbums;
			};

			const auto testCases = std::array {
				TestCase {createTrack("a", "album1", "a", "t1"), {{"album1", "a"}}},
				TestCase {createTrack("a", "album1", "a", "t2"), {{"album1", "a"}}},
				TestCase {createTrack("a", "album2", "a", "t3"), {{"album1", "a"}, {"album2", "a"}}},
				TestCase {createTrack("a", "album3", "a", "t4"), {{"album1", "a"}, {"album2", "a"}, {"album3", "a"}}},
				TestCase {createTrack("a", "album1", "other", "t5"),
				          {{"album1", "a"}, {"album1", "other"}, {"album2", "a"}, {"album3", "a"}}},
			};

			for(const auto& testCase: testCases)
			{
				const auto tracks = MetaDataList() << testCase.track;
				env.libraryDatabase->storeMetadata(tracks);

				auto albums = AlbumList {};
				env.libraryDatabase->getAllAlbums(albums, true);

				QCOMPARE(albums.count(), testCase.expectedAlbums.count());
				for(const auto& [name, albumArtist]: testCase.expectedAlbums)
				{
					const auto contains =
						Util::Algorithm::contains(albums, [n = name, a = albumArtist](const auto& album) {
							return (album.name() == n) && (album.albumArtist() == a);
						});

					QVERIFY(contains);
				}
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testInsertTracksAtOnceCreatesAlbums()
		{
			[[maybe_unused]] auto env = TestEnv {};

			const auto tracks = MetaDataList()
				<< createTrack("a", "album1", "a", "t1")
				<< createTrack("a", "album1", "a", "t2")
				<< createTrack("a", "album2", "a", "t3")
				<< createTrack("a", "album3", "a", "t4")
				<< createTrack("a", "album1", "other", "t5");

			const auto expectedAlbums = QList<QPair<QString, QString>> {
				{{"album1", "a"}, {"album1", "other"}, {"album2", "a"}, {"album3", "a"}}
			};

			env.libraryDatabase->storeMetadata(tracks);

			auto albums = AlbumList {};
			env.libraryDatabase->getAllAlbums(albums, true);

			QCOMPARE(albums.count(), expectedAlbums.count());
			for(const auto& [name, albumArtist]: expectedAlbums)
			{
				const auto contains =
					Util::Algorithm::contains(albums, [n = name, a = albumArtist](const auto& album) {
						return (album.name() == n) && (album.albumArtist() == a);
					});

				QVERIFY(contains);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testInsertTrackSetsArtistAndAlbumId()
		{
			[[maybe_unused]] auto env = TestEnv {};

			struct TestCase
			{
				MetaData track;
			};

			const auto testCases = std::array {
				TestCase {createTrack("ar1", "al1", "aa1", "t1")},
				TestCase {createTrack("ar1", "al2", "aa2", "t2")},
				TestCase {createTrack("ar1", "al3", "aa3", "t3")}
			};

			for(const auto& testCase: testCases)
			{
				const auto tracks = MetaDataList() << testCase.track;
				env.libraryDatabase->storeMetadata(tracks);

				auto fetchedTracks = MetaDataList {};
				env.libraryDatabase->getAllTracks(fetchedTracks);

				const auto it = Util::Algorithm::find(fetchedTracks, [&](const auto& track) {
					return testCase.track.isEqual(track);
				});

				QVERIFY(it->artist() == testCase.track.artist());
				QVERIFY(it->artistId() >= 0);
				QVERIFY(it->album() == testCase.track.album());
				QVERIFY(it->albumId() >= 0);
				QVERIFY(it->albumArtist() == testCase.track.albumArtist());
				QVERIFY(it->albumArtistId() >= 0);
			}
		}
};

QTEST_GUILESS_MAIN(LibraryDatabaseTest)

#include "LibraryDatabaseTest.moc"
