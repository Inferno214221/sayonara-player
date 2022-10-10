/* SmartPlaylistByYear.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "SmartPlaylistByYear.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language/Language.h"

#include <QDate>

SmartPlaylistByYear::SmartPlaylistByYear(const int id, const int fromYear, const int toYear, const bool isRandomized,
                                         const LibraryId libraryId) :
	SmartPlaylist(id, {fromYear, toYear}, isRandomized, libraryId) {}

SmartPlaylistByYear::~SmartPlaylistByYear() = default;

int SmartPlaylistByYear::minimumValue() const { return 1500; } // NOLINT(readability-magic-numbers)

int SmartPlaylistByYear::maximumValue() const { return QDate::currentDate().year(); }

MetaDataList SmartPlaylistByYear::filterTracks(MetaDataList tracks)
{
	const auto minYear = std::min(value(0), value(1));
	const auto maxYear = std::max(value(0), value(1));

	tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [&](const auto& track) {
		return (track.year() <= minYear) || (track.year() >= maxYear);
	}), tracks.end());

	return tracks;
}

QString SmartPlaylistByYear::classType() const { return SmartPlaylistByYear::ClassType; }

QString SmartPlaylistByYear::displayClassType() const { return Lang::get(Lang::Year); }

QString SmartPlaylistByYear::name() const
{
	const auto from = std::min(value(0), value(1));
	const auto to = std::max(value(0), value(1));
	return (from == to)
	       ? QString::number(from)
	       : QObject::tr("%1 - %2").arg(from).arg(to);
}

SmartPlaylists::Type SmartPlaylistByYear::type() const { return SmartPlaylists::Type::Year; }
