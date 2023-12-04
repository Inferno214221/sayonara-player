/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "Components/Bookmarks/BookmarkStorage.h"
#include "Components/Bookmarks/Bookmark.h"
#include "Database/Connector.h"
#include "Database/Bookmarks.h"
#include "Database/Module.h"
#include "Database/Query.h"
#include "Utils/MetaData/MetaData.h"

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	MetaData createDbTrack(const int id = 1)
	{
		auto track = MetaData {};
		track.setId(id);
		return track;
	}

	void deleteAllBookmarks()
	{
		auto deleteQuery = DB::Connector::instance()->runQuery("DELETE FROM savedbookmarks;", "Error");
		QVERIFY(!DB::hasError(deleteQuery));

		auto query = DB::Connector::instance()->runQuery("SELECT * FROM savedbookmarks;", "Error");
		QVERIFY(!DB::hasError(query));
		QVERIFY(!query.next());
	}
}

class BookmarkStorageTest :
	public Test::Base
{
	Q_OBJECT

	public:
		BookmarkStorageTest() :
			Test::Base("BookmarkStorageTest") {}

	private slots:

		[[maybe_unused]] void testInitiallyEmpty() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto bookmarkStorage = BookmarkStorage::create();

			QVERIFY(bookmarkStorage->count() == 0);
			QVERIFY(bookmarkStorage->bookmarks().isEmpty());
			QVERIFY(!bookmarkStorage->bookmark(3).isValid());
			QVERIFY(!bookmarkStorage->track().isValid());
		}

		[[maybe_unused]] void testCreateAndDeleteBookmarks() // NOLINT(readability-convert-member-functions-to-static)
		{
			deleteAllBookmarks();

			auto bookmarkStorage = BookmarkStorage::create(createDbTrack());
			for(Seconds i = 1; i <= 10; i++)// NOLINT(readability-magic-numbers)
			{
				auto creationStatus = bookmarkStorage->create(i);
				QVERIFY(creationStatus == BookmarkStorage::CreationStatus::Success);
				QVERIFY(bookmarkStorage->count() == i);
				QVERIFY(bookmarkStorage->bookmark(i - 1).timestamp() == i);
				QVERIFY(bookmarkStorage->bookmarks()[i - 1].timestamp() == i);
			}

			for(int i = 0; i < 10; i++)// NOLINT(readability-magic-numbers)
			{
				const auto success = bookmarkStorage->remove(0);
				QVERIFY(success);
				QVERIFY(bookmarkStorage->count() == 10 - i - 1);
			}
		}

		[[maybe_unused]] void
		testCreateBookmarkWithoutTrackFails() // NOLINT(readability-convert-member-functions-to-static)
		{
			deleteAllBookmarks();

			auto bookmarkStorage = BookmarkStorage::create();
			for(Seconds i = 1; i <= 10; i++)// NOLINT(readability-magic-numbers)
			{
				auto creationStatus = bookmarkStorage->create(i);
				QVERIFY(creationStatus == BookmarkStorage::CreationStatus::NoDBTrack);
				QVERIFY(bookmarkStorage->count() == 0);
			}
		}

		[[maybe_unused]] void
		testCreateSameBookmarkAgainFails() // NOLINT(readability-convert-member-functions-to-static)
		{
			deleteAllBookmarks();

			auto bookmarkStorage = BookmarkStorage::create(createDbTrack());
			const auto initialCreationStatus = bookmarkStorage->create(10);
			QVERIFY(initialCreationStatus == BookmarkStorage::CreationStatus::Success);
			QVERIFY(bookmarkStorage->count() == 1);

			const auto creationStatus = bookmarkStorage->create(10);
			QVERIFY(creationStatus == BookmarkStorage::CreationStatus::AlreadyThere);
			QVERIFY(bookmarkStorage->count() == 1);
		}

		[[maybe_unused]] void testCreateZeroBookmarkFails() // NOLINT(readability-convert-member-functions-to-static)
		{
			deleteAllBookmarks();

			auto bookmarkStorage = BookmarkStorage::create(createDbTrack());
			const auto initialCreationStatus = bookmarkStorage->create(0);
			QVERIFY(initialCreationStatus == BookmarkStorage::CreationStatus::OtherError);
			QVERIFY(bookmarkStorage->count() == 0);
		}

		[[maybe_unused]] void
		testBookmarkCountChangesWhenTrackChanges() // NOLINT(readability-convert-member-functions-to-static)
		{
			deleteAllBookmarks();

			struct Entry
			{
				int dbId;
				QList<Seconds> bookmarks;
			};

			const auto entries = std::array {
				Entry {1, {1}},
				Entry {2, {2}},
				Entry {3, {1, 3, 5, 7, 9}},
				Entry {4, {2, 4, 6, 8, 10}}};

			auto bookmarkStorage = BookmarkStorage::create();

			for(const auto& entry: entries)
			{
				bookmarkStorage->setTrack(createDbTrack(entry.dbId));
				QVERIFY(bookmarkStorage->count() == 0);

				for(const auto& timestamp: entry.bookmarks)
				{
					bookmarkStorage->create(timestamp);
				}

				QVERIFY(bookmarkStorage->count() == entry.bookmarks.count());
			}
		}
};

QTEST_GUILESS_MAIN(BookmarkStorageTest)

#include "BookmarkStorageTest.moc"
