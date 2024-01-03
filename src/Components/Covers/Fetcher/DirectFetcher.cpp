/* DirectFetcher.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "DirectFetcher.h"

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"

#include <QStringList>

using Cover::Fetcher::DirectFetcher;

DirectFetcher::DirectFetcher() :
	Cover::Fetcher::Base() {}

DirectFetcher::~DirectFetcher() = default;

QString DirectFetcher::privateIdentifier() const
{
	return "direct";
}

bool DirectFetcher::canFetchCoverDirectly() const
{
	return true;
}

QStringList DirectFetcher::parseAddresses([[maybe_unused]] const QByteArray& website) const
{
	return QStringList();
}

QString DirectFetcher::artistAddress([[maybe_unused]] const QString& artist) const
{
	return QString();
}

QString DirectFetcher::albumAddress([[maybe_unused]] const QString& artist, [[maybe_unused]] const QString& album) const
{
	return QString();
}

QString DirectFetcher::fulltextSearchAddress(const QString& str) const
{
	return (Util::File::isWWW(str) && Util::File::isImageFile(str))
	       ? str
	       : QString();
}

int DirectFetcher::estimatedSize() const
{
	return 1;
}

bool Cover::Fetcher::DirectFetcher::isWebserviceFetcher() const
{
	return false;
}

