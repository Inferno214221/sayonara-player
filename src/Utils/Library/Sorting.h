/* Sorting.h */

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

#ifndef LIBRARY_SORTING_H
#define LIBRARY_SORTING_H

#include "Sortorder.h"
#include "Utils/Settings/SettingConvertible.h"

class QString;
namespace Library
{
	/**
	 * @brief The Sortings class
	 * @ingroup LibraryHelper
	 */
	class Sortings :
		public SettingConvertible
	{
		public:
			SortOrder so_albums;
			SortOrder so_artists;
			SortOrder so_tracks;

		Sortings();
		Sortings(const Sortings& so);
		~Sortings();

		Sortings& operator=(const Sortings& other);
		bool operator==(Sortings so);

		QString toString() const override;
		bool loadFromString(const QString& str) override;
	};
}

#endif // SORTING_H
