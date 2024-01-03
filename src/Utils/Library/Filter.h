/* Filter.h

 * Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef SAYONARA_LIBRARY_FILTER_H
#define SAYONARA_LIBRARY_FILTER_H

#include "Utils/Pimpl.h"
#include "Utils/Library/SearchMode.h"

namespace Library
{
	class Filter
	{
		PIMPL(Filter)

		public:
			enum Mode
			{
				Fulltext = 0,
				Filename,
				Genre,
				InvalidGenre,
				Invalid
			};

			Filter();
			~Filter();

			Filter(const Filter& other);
			Filter& operator=(const Filter& other);

			void setFiltertext(const QString& filterText);
			QStringList filtertext(bool withPercent) const;
			QStringList searchModeFiltertext(bool withPercent, SearchModeMask searchModeMask) const;

			void setMode(Filter::Mode mode);
			Filter::Mode mode() const;

			void clear();
			bool cleared() const;
			bool isEqual(const Filter& other, int minimumSearchStringLength) const;
			int count() const;

			static QString filterModeName(Mode mode);
	};
}

Q_DECLARE_METATYPE(Library::Filter::Mode)

#endif /* SAYONARA_LIBRARY_FILTER_H */
