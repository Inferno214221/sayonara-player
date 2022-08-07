/* globals.h */

/* Copyright (C) 2012 Michael Lugmair (Lucio Carreras)
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

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <type_traits>

#ifndef CAST_MACROS
#define scast(x, y) static_cast<x>(y)
#define dcast(x, y) dynamic_cast<x>(y)
#define rcast(x, y) reinterpret_cast<x>(y)
#define CAST_MACROS
#endif

#define DARK_BLUE(x) QString("<font color=#0000FF>") + x + QString("</font>")
#define LIGHT_BLUE(x) QString("<font color=#8888FF>") + x + QString("</font>")

#define CAR_RET QString("<br />")
#define BOLD(x) QString("<b>") + x + QString("</b>")
#define BLACK(x) QString("<font color=#000000>") + x + QString("</font>")


// name, target, dark, string
#define LINK(n, t, d, s) if(d) s=QString("<a href=\"t\">)") + LIGHT_BLUE(n) + QString("</a>"); \
                         else  s=QString("<a href=\"t\">)") + DARK_BLUE(n) + QString("</a>");

#define SAYONARA_ORANGE_STR QString("#e8841a")
#define SAYONARA_ORANGE_COL QColor(232, 132, 26)

namespace Util
{
	template<typename TINT, typename T>
	typename std::enable_if<std::is_pointer<T>::value, bool>::type
	between(TINT idx, const T& cont)
	{
		return (idx >= 0 && idx < static_cast<TINT>(cont->size()));
	}

	template<typename TINT, typename T>
	typename std::enable_if<std::is_class<T>::value, bool>::type
	between(TINT idx, const T& cont)
	{
		return (idx >= 0 && idx < static_cast<TINT>(cont.size()));
	}

	template<typename TINT>
	typename std::enable_if<std::is_integral<TINT>::value, bool>::type
	between(TINT idx, TINT max)
	{
		return (idx >= 0 && idx < max);
	}

	enum SaveAsAnswer
	{
		Success,
		InvalidName,
		NotStorable,
		NameAlreadyThere,
		InvalidObject,
		OtherError
	};
}

template<typename T>
typename std::enable_if<std::is_enum<T>::value, typename std::underlying_type<T>::type>::type
constexpr operator+(T enumValue)
{
	return static_cast<typename std::underlying_type<T>::type>(enumValue);
}

#endif /* GLOBALS_H_ */
