/* CoverViewPixmapCache.cpp */

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

#include "CoverViewPixmapCache.h"
#include "Components/Covers/CoverLocation.h"

#include "Utils/Image.h"
#include "Utils/Set.h"

#include <QCache>
#include <QString>
#include <QPixmap>

using Library::CoverViewPixmapCache;

struct CoverViewPixmapCache::Private
{
	QCache<Hash, Util::Image> pixmaps;
	Util::Set<Hash> validHashes;
	QPixmap invalidCover;

	Private() :
		pixmaps {QCache<Hash, Util::Image>(1000)},
		invalidCover {Cover::Location::invalidPath()} {}
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

void CoverViewPixmapCache::addPixmap(const Hash& hash, const QPixmap& pixmap)
{
	if(!pixmap.isNull())
	{
		m->validHashes.insert(hash);
		m->pixmaps.insert(hash, new Util::Image(pixmap, QSize(200, 200)));
	}
}

QPixmap CoverViewPixmapCache::pixmap(const Hash& hash) const
{
	if(!m->pixmaps.contains(hash))
	{
		return QPixmap();
	}

	auto* img = m->pixmaps.object(hash);
	return (img != nullptr)
	       ? img->pixmap()
	       : QPixmap();
}

bool CoverViewPixmapCache::isOutdated(const Hash& hash) const
{
	return (!m->validHashes.contains(hash));
}

void CoverViewPixmapCache::setAllOutdated()
{
	m->validHashes.clear();
}

void CoverViewPixmapCache::setCacheSize(int cacheSize)
{
	if(m->pixmaps.maxCost() <= cacheSize)
	{
		m->pixmaps.setMaxCost(std::min(30, cacheSize));
	}
}

void CoverViewPixmapCache::clear()
{
	m->pixmaps.clear();
}
