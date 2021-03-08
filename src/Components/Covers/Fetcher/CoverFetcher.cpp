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

QString Cover::Fetcher::Base::artistAddress([[maybe_unused]] const QString& artist) const
{
	return QString();
}

QString
Cover::Fetcher::Base::albumAddress([[maybe_unused]] const QString& artist, [[maybe_unused]] const QString& album) const
{
	return QString();
}

QString Cover::Fetcher::Base::fulltextSearchAddress([[maybe_unused]] const QString& str) const
{
	return QString();
}

bool Cover::Fetcher::Base::isWebserviceFetcher() const
{
	return true;
}
