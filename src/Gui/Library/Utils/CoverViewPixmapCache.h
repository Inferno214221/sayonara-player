/* CoverViewPixmapCache.h */

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



#ifndef COVERVIEWPIXMAPCACHE_H
#define COVERVIEWPIXMAPCACHE_H

#include "Utils/Pimpl.h"
#include <QThread>

using Hash=QString;
class QPixmap;

class CoverViewPixmapCache
{
	PIMPL(CoverViewPixmapCache)

public:
	CoverViewPixmapCache();
	~CoverViewPixmapCache();

	bool has_pixmap(const Hash& hash) const;

	QPixmap pixmap(const Hash& hash) const;
	QPixmap invalid_pixmap() const;
	void	add_pixmap(const Hash& hash, const QPixmap& pm);
	int		cache_size() const;

	bool is_outdated(const Hash& hash) const;
	void set_cache_size(int cache_size);
	void clear();
};

#endif // COVERVIEWPIXMAPCACHE_H
