/* CoverViewPixmapCache.cpp */

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

#include "CoverViewPixmapCache.h"
#include "Components/Covers/CoverLocation.h"

#include "Utils/Utils.h"
#include "Utils/Image.h"
#include "Utils/Set.h"

#include <QCache>
#include <QString>
#include <QPixmap>

struct Pair
{
	Hash hash;
	QPixmap pm;

	Pair() {}
	Pair(Hash hash, QPixmap pm) : hash(hash), pm(pm)
	{}
};

struct CoverViewPixmapCache::Private
{
	QCache<Hash, Util::Image>	pixmaps;
	Util::Set<Hash>				valid_hashes;
	QPixmap						invalid_cover;

	Private()
	{
		pixmaps.setMaxCost(1000);
		invalid_cover = QPixmap(Cover::Location::invalid_location().preferred_path());
	}
};

CoverViewPixmapCache::CoverViewPixmapCache()
{
	m = Pimpl::make<Private>();
}

CoverViewPixmapCache::~CoverViewPixmapCache() {}


bool CoverViewPixmapCache::has_pixmap(const Hash& hash) const
{
	return m->pixmaps.contains(hash) && (m->pixmaps.object(hash) != nullptr);
}

QPixmap CoverViewPixmapCache::invalid_pixmap() const
{
	return m->invalid_cover;
}

void CoverViewPixmapCache::add_pixmap(const Hash& hash, const QPixmap& pm)
{
	if(pm.isNull()){
		return;
	}

	m->pixmaps.insert(hash, new Util::Image(pm, QSize(200, 200)));
}


QPixmap CoverViewPixmapCache::pixmap(const Hash& hash) const
{
	if(!m->pixmaps.contains(hash)){
		return QPixmap();
	}

	Util::Image* img = m->pixmaps.object(hash);
	if(!img){
		return QPixmap();
	}

	return img->pixmap();
}


bool CoverViewPixmapCache::is_outdated(const Hash& hash) const
{
	return (!m->valid_hashes.contains(hash));
}

void CoverViewPixmapCache::set_cache_size(int cache_size)
{
	m->pixmaps.setMaxCost(std::min(30, cache_size));
}
