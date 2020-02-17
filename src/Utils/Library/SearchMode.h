/* SearchMode.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

template<typename T>
class QList;
class QChar;
class QString;


namespace Library
{
	/**
	 * @brief The SearchMode enum
	 * @ingroup Library
	 * @ingroup Helper
	 */
	enum SearchMode
	{
		CaseInsensitve		= (1<<0),
		NoSpecialChars		= (1<<1),
		NoDiacriticChars	= (1<<2)
	};

	using SearchModeMask = int;

	namespace Utils
	{

		/**
		 * @brief Converts a user entered string into a cis-representation.
		 * For example, diacrytic chars are replaced by latin ones, spaces are removed
		 * and so on, so the resulting string can be searched for in the database
		 * @ingroup Library
		 * @ingroup Helper
		 * @param str source string
		 * @param mode combination of SearchMode values
		 */
		QString convertSearchstring(const QString& str, SearchModeMask mode);

		/**
		 * @brief Converts a user entered string into a cis-representation.
		 * For example, diacrytic chars are replaced by latin ones, spaces are removed
		 * and so on, so the resulting string can be searched for in the database
		 * @ingroup Library
		 * @ingroup Helper
		 * @param str source string
		 * @param mode combination of SearchMode values
		 * @param ignored_chars chars that are not replaced within that method
		 */
		QString convertSearchstring(const QString& str, SearchModeMask mode, const QList<QChar>& ignored_chars);
	}
}

#endif // LIBRARYSEARCHMODE_H
