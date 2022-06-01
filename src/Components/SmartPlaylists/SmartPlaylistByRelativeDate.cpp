/* SmartPlaylistByRelativeDate.cpp */
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

#include "SmartPlaylistByRelativeDate.h"
#include "TimeSpanConverter.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"
#include "Utils/Language/Language.h"

#include <QDateTime>
#include <QObject>

namespace
{
	constexpr const auto DaysPerYear = 365;
	constexpr const auto DaysPerMonth = 30;
	constexpr const auto MaxYears = 9;
	constexpr const auto MaxMonths = 11;
	constexpr const auto MaximumTimeSpan = DaysPerYear * MaxYears +
	                                       DaysPerMonth * MaxMonths +
	                                       DaysPerMonth - 1;

	bool isDateInsideRange(const MetaData& track, const int min, const int max)
	{
		// date = today - 100
		// minDate = today - 120
		// maxDate = today - 50

		const auto date = Util::intToDate(track.createdDate()).date();
		const auto today = QDate::currentDate();

		const auto minDate = today.addDays(-max);  // today - 14
		const auto maxDate = today.addDays(-min);  // today - 120

		return (date >= minDate) && (date <= maxDate);
	}
}

SmartPlaylistByRelativeDate::SmartPlaylistByRelativeDate(const int id, const int min, const int max) :
	SmartPlaylist {id, min, max} {}

SmartPlaylistByRelativeDate::~SmartPlaylistByRelativeDate() = default;

int SmartPlaylistByRelativeDate::minimumValue() const { return 0; }

int SmartPlaylistByRelativeDate::maximumValue() const { return MaximumTimeSpan; }

MetaDataList SmartPlaylistByRelativeDate::filterTracks(MetaDataList tracks)
{
	tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [&](const auto& track) {
		return !isDateInsideRange(track, from(), to());
	}), tracks.end());

	return tracks;
}

QString SmartPlaylistByRelativeDate::classType() const { return SmartPlaylistByRelativeDate::ClassType; }

QString SmartPlaylistByRelativeDate::displayClassType() const
{
	return QObject::tr("Age of tracks in days");
}

QString SmartPlaylistByRelativeDate::name() const
{
	const auto sc = stringConverter();
	if(from() == to())
	{
		return QObject::tr("%1 old").arg(sc->intToString(from()));
	}

	if(from() == 0)
	{
		return QObject::tr("Less than %1 old").arg(sc->intToString(to()));
	}

	if(to() == maximumValue())
	{
		return QObject::tr("Older than %1").arg(sc->intToString(from()));
	}

	return QObject::tr("Between %1 and %2 old")
		.arg(sc->intToString(from()))
		.arg(sc->intToString(to()));
}

SmartPlaylists::Type SmartPlaylistByRelativeDate::type() const { return SmartPlaylists::Type::CreatedRelative; }

SmartPlaylists::StringConverterPtr SmartPlaylistByRelativeDate::createConverter() const
{
	return std::make_shared<SmartPlaylists::TimeSpanConverter>();
}
