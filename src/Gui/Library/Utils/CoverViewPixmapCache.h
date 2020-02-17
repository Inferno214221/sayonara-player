/* CoverViewPixmapCache.h */

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

#ifndef COVERVIEWPIXMAPCACHE_H
#define COVERVIEWPIXMAPCACHE_H

#include "Utils/Pimpl.h"
#include <QThread>

using Hash=QString;
class QPixmap;

namespace Library
{
	/**
	 * @brief A cache class. Internally handled as QCache class
	 * @ingroup GuiLibrary
	 */
	class CoverViewPixmapCache
	{
		PIMPL(CoverViewPixmapCache)

		public:
			CoverViewPixmapCache();
			~CoverViewPixmapCache();

			bool hasPixmap(const Hash& hash) const;

			QPixmap pixmap(const Hash& hash) const;
			QPixmap invalidPixmap() const;
			void addPixmap(const Hash& hash, const QPixmap& pm);

			bool isOutdated(const Hash& hash) const;
			void setOutdated(const Hash& hash);
			void setAllOutdated();

			int	cacheSize() const;
			void setCacheSize(int cacheSize);

			void clear();
	};
}

#endif // COVERVIEWPIXMAPCACHE_H
