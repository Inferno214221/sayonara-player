/* Algorithm.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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



#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "globals.h"
#include "typedefs.h"
#include <algorithm>

namespace Util
{
	namespace Algorithm
	{
		template<typename T, typename FN>
		bool contains(const T& container, FN fn)
		{
			return std::any_of(container.begin(), container.end(), fn);
		}

		template<typename T, typename FN>
		void sort(T& container, FN fn)
		{
			std::sort(container.begin(), container.end(), fn);
		}

		template<typename T, typename FN>
		typename T::iterator find(T& container, FN fn)
		{
			return std::find_if(container.begin(), container.end(), fn);
		}

		template<typename T, typename FN>
		typename T::const_iterator find(const T& container, FN fn)
		{
			return std::find_if(container.begin(), container.end(), fn);
		}

		template<typename T>
		constexpr typename std::add_const<T>::type& AsConst(T& t) {
			return t;
		}

		template<typename T, typename FN>
		int indexOf(const T& container, FN fn) {
			auto it = Algorithm::find(container, fn);
			if(it == container.end())
			{
				return -1;
			}
			return std::distance(container.begin(), it);
		}
	}
}

#endif // ALGORITHM_H
