/* CoverFetcherInterface.cpp */

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

#include "CoverFetcher.h"
#include <QString>

Cover::Fetcher::Base::Base() = default;
Cover::Fetcher::Base::~Base() = default;

QString Cover::Fetcher::Base::identifier() const
{
	return this->privateIdentifier().toLower();
}

QString Cover::Fetcher::Base::artistAddress(const QString& artist) const
{
	Q_UNUSED(artist)
	return QString();
}

QString Cover::Fetcher::Base::albumAddress(const QString& artist, const QString& album) const
{
	Q_UNUSED(artist)
	Q_UNUSED(album)

	return QString();
}

QString Cover::Fetcher::Base::fulltextSearchAddress(const QString& str) const
{
	Q_UNUSED(str)

	return QString();
}
