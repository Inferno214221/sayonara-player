/* Filter.h

 * Copyright (C) 2011-2019 Lucio Carreras
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
				Genre,
				Track,
				Invalid
			};

			Filter();
			~Filter();

			Filter(const Filter& other);
			Filter& operator=(const Filter& other);

			bool operator==(const Filter& other);

			/**
			 * @brief get splitted filtertext with or without percent. Needed for file search
			 * for example where cissearch is not suitable
			 * @param with_percent appends and prepends a percent sign to each filtertext
			 * @return
			 */
			QStringList filtertext(bool with_percent) const;

			/**
			 * @brief get splitted filtertext with or without percent
			 * but converts the search string suitable for cissearch first.
			 * @param with_percent appends and prepends a percent sign to each filtertext
			 * @return
			 */
			QStringList search_mode_filtertext(bool with_percent) const;


			/**
			 * @brief set comma separated filtertext. Usually this is the string
			 * found at the search box in the library
			 * @param str searchstring
			 * @param search_mode what do we want to search?
			 */
			void set_filtertext(const QString& str, SearchModeMask search_mode);

			Filter::Mode mode() const;
			void set_mode(Filter::Mode mode);

			/**
			 * @brief Clear the searchterm. We are not looking for an invalid genre, too
			 */
			void clear();

			/**
			 * @brief Is the searchterm empty?
			 * @return true if the searchterm is empty. If the current mode indicates
			 * that we are looking for an invalid genre, false is returned, too
			 */
			bool cleared() const;

			void set_invalid_genre(bool b);
			bool is_invalid_genre() const;

			bool is_usable() const;

			TrackID track_id() const;

			/**
			 * @brief Get an human readable string for the mode
			 * @param mode
			 * @return
			 */
			static QString get_text(Mode mode);
	};
}

#endif /* FILTER_H_ */
