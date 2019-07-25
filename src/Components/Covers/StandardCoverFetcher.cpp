/* StandardCoverFetcher.cpp */

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

#include "StandardCoverFetcher.h"
#include <QStringList>

using namespace Cover::Fetcher;

bool Standard::can_fetch_cover_directly() const
{
    return true;
}

QStringList Standard::calc_addresses_from_website(const QByteArray& website) const
{
	Q_UNUSED(website)
    return QStringList();
}


QString Standard::artist_address(const QString& artist) const
{
	Q_UNUSED(artist)
	return QString();
}

QString Standard::album_address(const QString& artist, const QString& album) const
{
	Q_UNUSED(artist)
	Q_UNUSED(album)
	return QString();
}

QString Standard::search_address(const QString& str) const
{
	Q_UNUSED(str)
	return QString();
}

bool Standard::is_search_supported() const
{
	return false;
}

bool Standard::is_album_supported() const
{
	return false;
}

bool Standard::is_artist_supported() const
{
	return false;
}


int Standard::estimated_size() const
{
	return -1;
}

QString Standard::identifier() const
{
	return QString();
}
