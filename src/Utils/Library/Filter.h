/* Filter.h

 * Copyright (C) 2011-2017 Lucio Carreras
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Lucio Carreras,
 * Jul 9, 2012
 *
 */

#ifndef FILTER_H_
#define FILTER_H_


#include "Utils/Pimpl.h"
#include "Utils/Library/SearchMode.h"

class QString;

namespace Library
{
	class DateFilter;
	/**
	 * @brief The Filter class
	 * @ingroup LibraryHelper
	 */
	class Filter
	{
		PIMPL(Filter)

		public:

			enum Mode
			{
				Fulltext=0,
				Filename,
				Genre
			};


			Filter();
			~Filter();

			Filter(const Filter& other);
			Filter& operator=(const Filter& other);

			bool operator==(const Filter& other);

			QString filtertext(bool with_percent) const;
			QString search_mode_filtertext(bool with_percent) const;
			void set_filtertext(const QString& str, SearchModeMask search_mode);

			Filter::Mode mode() const;
			void set_mode(Filter::Mode mode);

			void clear();
			bool cleared() const;

			static QString get_text(Mode mode);
	};
}

#endif /* FILTER_H_ */
