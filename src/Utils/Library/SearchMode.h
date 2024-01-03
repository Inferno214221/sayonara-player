/* SearchMode.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef LIBRARYSEARCHMODE_H
#define LIBRARYSEARCHMODE_H

#include <type_traits>

template<typename T>
class QList;
class QChar;
class QString;

namespace Library
{
	enum class SearchMode :
		int
	{
		None = 0,
		CaseInsensitve = (1 << 0),
		NoSpecialChars = (1 << 1),
		NoDiacriticChars = (1 << 2),
		SearchModeMaskSize = (1 << 3)
	};

	using SearchModeMask = std::underlying_type_t<SearchMode>;

	QString convertSearchstring(const QString& str);
	QString convertSearchstring(const QString& str, SearchModeMask mode);
	QString convertSearchstring(const QString& str, SearchModeMask mode, const QList<QChar>& ignoredChars);
}

#endif // LIBRARYSEARCHMODE_H
