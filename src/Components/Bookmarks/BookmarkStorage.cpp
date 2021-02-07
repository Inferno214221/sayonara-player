/* BookmarkStorage.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "BookmarkStorage.h"
#include "Bookmark.h"

#include "Database/Connector.h"
#include "Database/Bookmarks.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaData.h"

namespace Algorithm = Util::Algorithm;

namespace
{
	void sortBookmarks(QList<Bookmark>& bookmarks)
	{
		Algorithm::sort(bookmarks, [](const auto& bm1, const auto& bm2) {
			return (bm1.timestamp() < bm2.timestamp());
		});
	}
}

struct BookmarkStorage::Private
{
	QList<Bookmark> bookmarks;
	MetaData track;
	DB::Bookmarks* db;

	Private(const MetaData& track) :
		track {track},
		db {DB::Connector::instance()->bookmarkConnector()}
	{
		reloadBookmarks();
	}

	void reloadBookmarks()
	{
		bookmarks.clear();

		if(!track.customField("Chapter1").isEmpty())
		{
			int chapterIndex = 1;

			QString entry;

			do
			{
				const auto customFieldName = QString("Chapter%1").arg(chapterIndex);
				entry = track.customField(customFieldName);

				auto entries = entry.split(":");
				const auto length = static_cast<Seconds>(entries.takeFirst().toInt());
				const auto name = entries.join(":");

				bookmarks << Bookmark(length, name, true);
				chapterIndex++;

			} while(!entry.isEmpty());
		}

		if(bookmarks.isEmpty() && track.id() >= 0)
		{
			QMap<Seconds, QString> bookmarkMap;
			db->searchBookmarks(track.id(), bookmarkMap);

			for(auto it = bookmarkMap.begin(); it != bookmarkMap.end(); it++)
			{
				bookmarks << Bookmark {it.key(), it.value(), true};
			}
		}

		sortBookmarks(bookmarks);
	}
};

BookmarkStorage::BookmarkStorage() :
	BookmarkStorage{MetaData{}}
{}

BookmarkStorage::BookmarkStorage(const MetaData& track)
{
	m = Pimpl::make<Private>(track);
}

BookmarkStorage::~BookmarkStorage() = default;

BookmarkStorage::CreationStatus BookmarkStorage::create(Seconds timestamp)
{
	if(m->track.id() < 0 || m->track.databaseId() != 0)
	{
		return CreationStatus::NoDBTrack;
	}

	if(timestamp == 0)
	{
		return CreationStatus::OtherError;
	}

	const auto alreadyThere = Algorithm::contains(m->bookmarks, [&timestamp](const Bookmark& bm) {
		return (bm.timestamp() == timestamp);
	});

	if(alreadyThere)
	{
		return CreationStatus::AlreadyThere;
	}

	const auto name = Util::msToString(timestamp * 1000, "$M:$S");
	const auto success = m->db->insertBookmark(m->track.id(), timestamp, name);
	if(success)
	{
		m->reloadBookmarks();
		return CreationStatus::Success;
	}

	return CreationStatus::DBError;
}

bool BookmarkStorage::remove(int idx)
{
	if(!Util::between(idx, count()))
	{
		return false;
	}

	const auto removed = m->db->removeBookmark(m->track.id(), m->bookmarks[idx].timestamp());
	if(removed)
	{
		m->reloadBookmarks();
	}

	return removed;
}

const QList<Bookmark>& BookmarkStorage::bookmarks() const
{
	return m->bookmarks;
}

Bookmark BookmarkStorage::bookmark(int index) const
{
	return (Util::between(index, count()))
		? m->bookmarks[index]
		: Bookmark{};
}

int BookmarkStorage::count() const
{
	return m->bookmarks.count();
}

void BookmarkStorage::setTrack(const MetaData& track)
{
	m->track = track;
	m->reloadBookmarks();
}

const MetaData& BookmarkStorage::track() const
{
	return m->track;
}
