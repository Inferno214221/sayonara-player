/* CoverViewPixmapCache.cpp */

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

#include "CoverViewPixmapCache.h"
#include "Components/Covers/CoverLocation.h"

#include "Utils/Utils.h"
#include "Utils/Image.h"
#include "Utils/Set.h"

#include <QCache>
#include <QString>
#include <QPixmap>

using Library::CoverViewPixmapCache;

struct Pair
{
	Hash hash;
	QPixmap pm;

	Pair() = default;

	Pair(Hash hash, QPixmap pm) :
		hash(hash), pm(pm) {}
};

struct CoverViewPixmapCache::Private
{
	QCache<Hash, Util::Image> pixmaps;
	Util::Set<Hash> validHashes;
	QPixmap invalidCover;

	Private()
	{
		pixmaps.setMaxCost(1000);
		invalidCover = QPixmap(Cover::Location::invalidPath());
	}
};

CoverViewPixmapCache::CoverViewPixmapCache()
{
	m = Pimpl::make<Private>();
}

CoverViewPixmapCache::~CoverViewPixmapCache() = default;

bool CoverViewPixmapCache::hasPixmap(const Hash& hash) const
{
	return m->pixmaps.contains(hash) && (m->pixmaps.object(hash) != nullptr);
}

QPixmap CoverViewPixmapCache::invalidPixmap() const
{
	return m->invalidCover;
}

void CoverViewPixmapCache::addPixmap(const Hash& hash, const QPixmap& pm)
{
	if(pm.isNull())
	{
		return;
	}

	m->validHashes.insert(hash);
	m->pixmaps.insert(hash, new Util::Image(pm, QSize(200, 200)));
}

int CoverViewPixmapCache::cacheSize() const
{
	return m->pixmaps.keys().size();
}

QPixmap CoverViewPixmapCache::pixmap(const Hash& hash) const
{
	if(!m->pixmaps.contains(hash))
	{
		return QPixmap();
	}

	auto* img = m->pixmaps.object(hash);
	if(!img)
	{
		return QPixmap();
	}

	return img->pixmap();
}

bool CoverViewPixmapCache::isOutdated(const Hash& hash) const
{
	return (!m->validHashes.contains(hash));
}

void CoverViewPixmapCache::setOutdated(const Hash& hash)
{
	m->validHashes.remove(hash);
}

void CoverViewPixmapCache::setAllOutdated()
{
	m->validHashes.clear();
}

void CoverViewPixmapCache::setCacheSize(int cache_size)
{
	if(m->pixmaps.maxCost() > cache_size)
	{
		return;
	}

	m->pixmaps.setMaxCost(std::min(30, cache_size));
}

void CoverViewPixmapCache::clear()
{
	m->pixmaps.clear();
}
