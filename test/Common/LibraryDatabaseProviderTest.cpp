/*
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

#include "Common/SayonaraTest.h"
#include "Common/LibraryDatabaseProvider.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Album.h"

// access working directory with Test::Base::tempPath("somefile.txt");

class LibraryDatabaseProviderTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LibraryDatabaseProviderTest() :
			Test::Base("LibraryDatabaseProviderTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testDbIsValid()
		{
			const auto provider = Test::LibraryDatabaseProvider(
				0, tempPath(), {}, ::DB::ArtistIdInfo::ArtistIdField::ArtistId);

			auto* db = provider.libraryDatabase();
			QVERIFY(db != nullptr);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testTracksAreWrittenAndDeleted()
		{
			struct TestCase
			{
				QList<Test::MetaDataBlock> trackBlocks;
				QStringList expectedTracks;
			};

			const auto testCases = std::array {
				TestCase {
					{
						{"album1", "artist1", "a"},
						{"album1", "artist1", "b"},
					},
					{
						tempPath("artist1/album1/a.mp3"),
						tempPath("artist1/album1/b.mp3"),
					}
				},
				TestCase {
					{
						{"album2", "artist2", "c"},
						{"album2", "artist2", "d"}
					},
					{
						tempPath("artist2/album2/c.mp3"),
						tempPath("artist2/album2/d.mp3")
					}
				}
			};

			for(const auto& testCase: testCases)
			{
				constexpr const auto artistIdField = ::DB::ArtistIdInfo::ArtistIdField::ArtistId;
				const auto provider = Test::LibraryDatabaseProvider(0, tempPath(), testCase.trackBlocks, artistIdField);

				auto* db = provider.libraryDatabase();

				auto tracks = MetaDataList {};
				db->getAllTracks(tracks);

				auto filenames = QStringList {};
				Util::Algorithm::transform(tracks, filenames, [](const auto& track) {
					return track.filepath();
				});

				filenames.sort();

				QCOMPARE(filenames, testCase.expectedTracks);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testArtistsAndAlbumsAreWrittenAndCleared()
		{
			struct TestCase
			{
				QList<Test::MetaDataBlock> trackBlocks;
				QStringList expectedArtists;
				QStringList expectedAlbums;
			};

			const auto testCases = std::array {
				TestCase {
					{
						{"album1", "artist1", "a"},
						{"album1", "artist1", "b"},
					},
					QStringList {"artist1"},
					QStringList {"album1"}
				},
				TestCase {
					{
						{"album2", "artist2", "d"},
						{"album2", "artist2", "e"}
					},
					QStringList {"artist2"},
					QStringList {"album2"}
				},
				TestCase {
					{
						{"album4", "artist4", "x"},
						{"album5", "artist5", "y"},
						{"album6", "artist6", "z"}
					},
					QStringList {"artist4", "artist5", "artist6"},
					QStringList {"album4", "album5", "album6"}
				}
			};

			for(const auto& testCase: testCases)
			{
				constexpr const auto artistIdField = ::DB::ArtistIdInfo::ArtistIdField::ArtistId;
				const auto provider = Test::LibraryDatabaseProvider(0, tempPath(), testCase.trackBlocks, artistIdField);

				auto* db = provider.libraryDatabase();

				auto artists = ArtistList {};
				db->getAllArtists(artists, true);
				auto artistNames = QStringList {};
				Util::Algorithm::transform(artists, artistNames, [](const auto& artist) { return artist.name(); });
				artistNames.sort();
				QCOMPARE(artistNames, testCase.expectedArtists);

				auto albums = AlbumList {};
				db->getAllAlbums(albums, true);
				auto albumNames = QStringList {};
				Util::Algorithm::transform(albums, albumNames, [](const auto& album) { return album.name(); });
				albumNames.sort();
				QCOMPARE(albumNames, testCase.expectedAlbums);
			}
		}
};

QTEST_GUILESS_MAIN(LibraryDatabaseProviderTest)

#include "LibraryDatabaseProviderTest.moc"
