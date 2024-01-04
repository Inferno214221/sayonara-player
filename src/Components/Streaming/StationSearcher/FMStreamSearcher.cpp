/* FMStreamSearcher.cpp, (Created on 03.01.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of Sayonara Player
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

#include "FMStreamSearcher.h"
#include "FMStreamParser.h"

#include <QString>

QString FMStreamSearcher::buildUrl(const QString& searchtext, const StationSearcher::Mode mode, const int page,
                                   const int /*maxEntries*/) const
{
	if(mode == StationSearcher::Style)
	{
		return QString("http://fmstream.org/index.php?style=%1")
			.arg(searchtext);
	}

	if(page == 0)
	{
		return QString("http://fmstream.org/index.php?s=%1")
			.arg(searchtext);
	}

	return QString("http://fmstream.org/index.php?s=%1&n=%2")
		.arg(searchtext)
		.arg(page);
}

std::unique_ptr<StationParser> FMStreamSearcher::createStationParser()
{
	return std::make_unique<FMStreamParser>();
}
