/* Bookmark.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include "Utils/Pimpl.h"

/**
 * @brief The Bookmark helper class
 * @ingroup Bookmarks
 */
class QString;
class Bookmark
{
	PIMPL(Bookmark)

	public:
		explicit Bookmark(Seconds timestamp, const QString& name, bool valid=false);
		explicit Bookmark(Seconds timestamp=0);

		Bookmark(const Bookmark& other);
		Bookmark& operator=(const Bookmark &other);

		~Bookmark();

		Seconds	timestamp() const;
		QString			name() const;
		bool			is_valid() const;
};


#endif // BOOKMARK_H
