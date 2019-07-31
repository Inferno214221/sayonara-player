/* ActionPair.h */

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

#ifndef ACTIONPAIR_H
#define ACTIONPAIR_H

#include <QString>
#include "Utils/Library/Sortorder.h"
#include "Utils/Language/Language.h"
#include "Utils/Pimpl.h"

namespace Library
{
	/**
	 * @brief A mapping between a name and a sortorder
	 */
	class ActionPair
	{
		PIMPL(ActionPair)

		public:

			/**
			 * @brief appends "Ascending" or "Descending" behind the name
			 * @param t1 a term like e.g. 'Lang::Title'
			 * @param ascending indicates if the so is ascending or descending
			 * @param so
			 */
			ActionPair(Lang::Term term, bool ascending, Library::SortOrder so);
			ActionPair(const ActionPair& other);
			ActionPair& operator=(const ActionPair& other);

			~ActionPair();

			QString name() const;
			Library::SortOrder sortorder() const;
	};
}

#endif // ACTIONPAIR_H
