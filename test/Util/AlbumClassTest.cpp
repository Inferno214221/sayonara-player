/* AlbumClassTest.cpp
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

#include "Utils/MetaData/Album.h"

namespace
{
	Album createTestAlbum()
	{
		Album album;

		album.setAlbumArtist("Album3");
		album.setArtists({"artist1", "artist2"});
		album.setCoverDownloadUrls({"http://bla.jpg", "https://bla.jpg"});
		album.setDatabaseId(2);
		album.setDiscnumbers({0, 3, 2, 3});
		album.setDurationSec(150); // NOLINT(readability-magic-numbers)
		album.setId(4);
		album.setName("Album1");
		album.setPathHint({"/some/hint/to/path"});
		album.setRating(Rating::Three);
		album.setSongcount(4);
		album.setYear(2005); // NOLINT(readability-magic-numbers)

		return album;
	}

	void verify(const Album& album) // NOLINT(readability-function-cognitive-complexity)
	{
		QVERIFY(album.albumArtist() == "Album3");
		QVERIFY(album.artists().contains("artist1"));
		QVERIFY(album.artists().contains("artist2"));
		QVERIFY(album.artists().count() == 2);
		QVERIFY(album.databaseId() == 2);
		QVERIFY(album.disccount() == 3);
		QVERIFY(album.durationSec() == 150);
		QVERIFY(album.id() == 4);
		QVERIFY(album.isSampler() == true);
		QVERIFY(album.name() == "Album1");
		QVERIFY(album.rating() == Rating::Three);
		QVERIFY(album.songcount() == 4);
		QVERIFY(album.year() == 2005);
	}
}

class AlbumClassTest :
	public Test::Base
{
	Q_OBJECT

	public:
		AlbumClassTest() :
			Test::Base("MDAlbumTest") {}

		~AlbumClassTest() override = default;

	private slots:
		[[maybe_unused]] void testConstructor();
		[[maybe_unused]] void testCopyConstructor();
		[[maybe_unused]] void testAssignment();
		[[maybe_unused]] void testMoveConstructor();
		[[maybe_unused]] void testMoveAssignment();
};

[[maybe_unused]] void AlbumClassTest::testConstructor() // NOLINT(readability-convert-member-functions-to-static)
{
	const auto album = createTestAlbum();
	verify(album);
}

[[maybe_unused]] void AlbumClassTest::testCopyConstructor() // NOLINT(readability-convert-member-functions-to-static)
{
	const auto album = createTestAlbum();
	auto album2 = Album {album};
	QVERIFY(album2.uniqueId() != album.uniqueId());
}

[[maybe_unused]] void AlbumClassTest::testAssignment() // NOLINT(readability-convert-member-functions-to-static)
{
	const auto album = createTestAlbum();
	const auto album2 = album; // NOLINT(performance-unnecessary-copy-initialization)
	QVERIFY(album2.uniqueId() != album.uniqueId());
	verify(album2);
}

[[maybe_unused]] void AlbumClassTest::testMoveConstructor() // NOLINT(readability-convert-member-functions-to-static)
{
	auto album = createTestAlbum();
	const auto uniqueId = album.uniqueId();

	const Album album2(std::move(album));
	verify(album2);

	QVERIFY(album2.uniqueId() == uniqueId);

	album = album2; // test if album is still assignable
	QVERIFY(album.uniqueId() != album2.uniqueId());
}

[[maybe_unused]] void AlbumClassTest::testMoveAssignment() // NOLINT(readability-convert-member-functions-to-static)
{
	auto album = createTestAlbum();
	const auto uniqueId = album.uniqueId();

	const auto album2 = std::move(album);
	verify(album2);

	QVERIFY(album2.uniqueId() == uniqueId);

	album = album2; // test if album is still assignable
	QVERIFY(album.uniqueId() != album2.uniqueId());
}

QTEST_GUILESS_MAIN(AlbumClassTest)

#include "AlbumClassTest.moc"

