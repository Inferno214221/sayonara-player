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

#include "Database/Bookmarks.h"
#include "Database/Connector.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Utils.h"

#include <utility>

namespace
{
	void sortBookmarks(QList<Bookmark>& bookmarks)
	{
		Util::Algorithm::sort(bookmarks, [](const auto& bm1, const auto& bm2) {
			return (bm1.timestamp() < bm2.timestamp());
		});
	}

	class BookmarkStorageImpl :
		public BookmarkStorage
	{
		public:
			explicit BookmarkStorageImpl(MetaData track) :
				m_track {std::move(track)}
			{
				reloadBookmarks();
			}

			~BookmarkStorageImpl() override = default;

			CreationStatus create(const Seconds timestamp) override
			{
				if((m_track.id() < 0) || (m_track.databaseId() != 0))
				{
					return CreationStatus::NoDBTrack;
				}

				if(timestamp == 0)
				{
					return CreationStatus::OtherError;
				}

				const auto alreadyThere = Util::Algorithm::contains(m_bookmarks, [&timestamp](const auto& bookmark) {
					return (bookmark.timestamp() == timestamp);
				});

				if(alreadyThere)
				{
					return CreationStatus::AlreadyThere;
				}

				const auto name = Util::msToString(timestamp * 1000, "$M:$S");
				const auto success = m_db->insertBookmark(m_track.id(), timestamp, name);
				if(success)
				{
					reloadBookmarks();
					return CreationStatus::Success;
				}

				return CreationStatus::DBError;
			}

			bool remove(const int index) override
			{
				if(!Util::between(index, count()))
				{
					return false;
				}

				const auto removed = m_db->removeBookmark(m_track.id(), m_bookmarks[index].timestamp());
				if(removed)
				{
					reloadBookmarks();
				}

				return removed;
			}

			[[nodiscard]] const QList<Bookmark>& bookmarks() const override { return m_bookmarks; }

			[[nodiscard]] Bookmark bookmark(const int index) const override
			{
				return Util::between(index, count())
				       ? m_bookmarks[index]
				       : Bookmark {};
			}

			[[nodiscard]] int count() const override { return m_bookmarks.count(); }

			void setTrack(const MetaData& track) override
			{
				m_track = track;
				reloadBookmarks();
			}

			[[nodiscard]] const MetaData& track() const override { return m_track; }

		private:
			void reloadBookmarks()
			{
				m_bookmarks.clear();

				if(!m_track.customField("Chapter1").isEmpty())
				{
					for(auto chapterIndex = 1;; chapterIndex++)
					{
						const auto customFieldName = QString("Chapter%1").arg(chapterIndex);
						const auto entry = m_track.customField(customFieldName);
						if(entry.isEmpty())
						{
							break;
						}

						auto entries = entry.split(":");
						const auto length = static_cast<Seconds>(entries.takeFirst().toInt());
						const auto name = entries.join(":");

						m_bookmarks << Bookmark(length, name, true);
					}
				}

				if(m_bookmarks.isEmpty() && m_track.id() >= 0)
				{
					QMap<Seconds, QString> bookmarkMap;
					m_db->searchBookmarks(m_track.id(), bookmarkMap);

					for(auto it = bookmarkMap.begin(); it != bookmarkMap.end(); it++)
					{
						m_bookmarks << Bookmark {it.key(), it.value(), true};
					}
				}

				sortBookmarks(m_bookmarks);
			}

			QList<Bookmark> m_bookmarks;
			MetaData m_track;
			DB::Bookmarks* m_db {DB::Connector::instance()->bookmarkConnector()};
	};
}

BookmarkStorage::~BookmarkStorage() = default;

BookmarkStoragePtr BookmarkStorage::create(const MetaData& track)
{
	return std::make_shared<BookmarkStorageImpl>(track);
}
