/* Filter.h

 * Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
 * created by Michael Lugmair (Lucio Carreras),
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
	 * @ingroup Library
	 * @ingroup Helper
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
				Invalid
			};

			Filter();
			~Filter();

			Filter(const Filter& other);
			Filter& operator=(const Filter& other);

			bool operator==(const Filter& other);

			int count() const;

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
			QStringList searchModeFiltertext(bool with_percent) const;


			/**
			 * @brief set comma separated filtertext. Usually this is the string
			 * found at the search box in the library
			 * @param str searchstring
			 * @param search_mode what do we want to search?
			 */
			void setFiltertext(const QString& str, SearchModeMask search_mode);

			/**
			 * @brief Returns the filtermode
			 * @return
			 */
			Filter::Mode mode() const;

			/**
			 * @brief Sets the Filter::Mode.
			 * @param mode
			 */
			void setMode(Filter::Mode mode);

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

			/**
			 * @brief Sets a genre, which is not searched directly. This is
			 * meant to fetch all tracks which contains no genre
			 * @param b
			 */
			void setInvalidGenre(bool b);

			/**
			 * @brief Is the invalid genre mode active?
			 * @return
			 */
			bool isInvalidGenre() const;

			/**
			 * @brief Invalid mode is not usable.
			 * Invalid genre means, that this is a valid query -> Usable.
			 * When searching for a track, the id needs to be non-negative.
			 * When searching for different stuff, the size of the searchstrings
			 * needs at least to be 3 characters
			 * @return
			 */
			bool isUseable() const;

			/**
			 * @brief Get an human readable string for the mode
			 * @param mode
			 * @return
			 */
			static QString text(Mode mode);
	};
}

#endif /* FILTER_H_ */
