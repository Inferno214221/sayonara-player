/* SearchInformation.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef SEARCHINFORMATION_H
#define SEARCHINFORMATION_H

#include "Utils/Pimpl.h"

class QString;

namespace SC
{
	class SearchInformation
	{
		PIMPL(SearchInformation)

		public:
			SearchInformation(int artist_id, int album_id, int track_id, const QString& search_string);
			~SearchInformation();

			QString search_string() const;
			int artist_id() const;
			int album_id() const;
			int track_id() const;
	};

	class SearchInformationList
	{
		PIMPL(SearchInformationList)

		public:
			SearchInformationList();
			~SearchInformationList();

			IntSet artist_ids(const QString& search_string) const;
			IntSet album_ids(const QString& search_string) const;
			IntSet track_ids(const QString& search_string) const;

			SearchInformationList& operator<<(const SearchInformation& search_information);
			bool is_empty() const;
			void clear();
	};
}

#endif // SEARCHINFORMATION_H
