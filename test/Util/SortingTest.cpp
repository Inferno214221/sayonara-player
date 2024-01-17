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
#include "Utils/Library/Sorting.h"

// access working directory with Test::Base::tempPath("somefile.txt");

using Library::AlbumSortorder;
using Library::ArtistSortorder;
using Library::TrackSortorder;

namespace
{
	constexpr const auto DefaultAlbumSortorder = AlbumSortorder::NameAsc;
	constexpr const auto DefaultArtistSortorder = ArtistSortorder::NameAsc;
	constexpr const auto DefaultTrackSortorder = TrackSortorder::AlbumAsc;
}

class SortingTest :
	public Test::Base
{
	Q_OBJECT

	public:
		SortingTest() :
			Test::Base("SortingTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testDefaultValue()
		{
			const auto sorting = Library::Sortings();
			QVERIFY(sorting.album == DefaultAlbumSortorder);
			QVERIFY(sorting.artist == DefaultArtistSortorder);
			QVERIFY(sorting.tracks == DefaultTrackSortorder);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testConvertToString()
		{
			struct TestCase
			{
				AlbumSortorder albumSortorder;
				ArtistSortorder artistSortorder;
				TrackSortorder trackSortorder;
				QString expectedString;
			};

			const auto testCases = std::array {
				TestCase {AlbumSortorder::NameAsc, ArtistSortorder::NameDesc, TrackSortorder::TitleAsc, "1,2,3"},
				TestCase {AlbumSortorder::Last, ArtistSortorder::Last, TrackSortorder::Last, "15,5,31"},
				TestCase {AlbumSortorder::NoSorting, ArtistSortorder::NoSorting, TrackSortorder::NoSorting, "0,0,0"}
			};

			for(const auto& testCase: testCases)
			{
				auto sorting = Library::Sortings {};
				sorting.album = testCase.albumSortorder;
				sorting.artist = testCase.artistSortorder;
				sorting.tracks = testCase.trackSortorder;

				const auto result = sorting.toString();
				QVERIFY(sorting.toString() == testCase.expectedString);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testConvertFromString()
		{
			struct TestCase
			{
				QString str;
				bool success;
				AlbumSortorder albumSortorder;
				ArtistSortorder artistSortorder;
				TrackSortorder trackSortorder;
			};

			// @formatter:off
			const auto testCases = std::array {
				TestCase {"1,2,3", true, AlbumSortorder::NameAsc, ArtistSortorder::NameDesc, TrackSortorder::TitleAsc},
				TestCase {"1,2,3,4", true, AlbumSortorder::NameAsc, ArtistSortorder::NameDesc, TrackSortorder::TitleAsc},
				TestCase {"2,3", false, DefaultAlbumSortorder, DefaultArtistSortorder, DefaultTrackSortorder},
				TestCase {"", false, DefaultAlbumSortorder, DefaultArtistSortorder, DefaultTrackSortorder},
				TestCase {"100,2,3", true, DefaultAlbumSortorder, ArtistSortorder::NameDesc, TrackSortorder::TitleAsc},
				TestCase {"100,100,100", true, DefaultAlbumSortorder, DefaultArtistSortorder, DefaultTrackSortorder},
				TestCase {"-1,-1,-1", true, DefaultAlbumSortorder, DefaultArtistSortorder, DefaultTrackSortorder},
			};
			// @formatter:on

			for(const auto& testCase: testCases)
			{
				auto sorting = Library::Sortings {};
				const auto success = sorting.loadFromString(testCase.str);

				QVERIFY(success == testCase.success);
				QVERIFY(sorting.album == testCase.albumSortorder);
				QVERIFY(sorting.artist == testCase.artistSortorder);
				QVERIFY(sorting.tracks == testCase.trackSortorder);
			}
		}
};

QTEST_GUILESS_MAIN(SortingTest)

#include "SortingTest.moc"
