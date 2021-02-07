/* BookmarkStorage.h */

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

#ifndef BOOKMARK_STORAGE_H
#define BOOKMARK_STORAGE_H

#include "Utils/Pimpl.h"
#include "Bookmark.h"

class MetaData;
class BookmarkStorage
{
	PIMPL(BookmarkStorage)

	public:
		enum class CreationStatus : unsigned char
		{
				Success,
				AlreadyThere,
				NoDBTrack,
				DBError,
				OtherError
		};

		BookmarkStorage();
		BookmarkStorage(const MetaData& track);

		virtual ~BookmarkStorage();

		/**
		 * @brief create a new bookmark for current track and current position
		 * @return true if successful, else false
		 */
		virtual CreationStatus create(Seconds timestamp);

		/**
		 * @brief remove single bookmark from database for current track
		 * @param idx index
		 * @return
		 */
		bool remove(int index);

		const QList<Bookmark>& bookmarks() const;
		Bookmark bookmark(int index) const;

		int count() const;

		void setTrack(const MetaData& track);
		const MetaData& track() const;
};

#endif // BOOKMARK_STORAGE_H
