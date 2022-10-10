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
	constexpr const auto DaysPerYear = 360;
	constexpr const auto MaxYears = 10;

	bool isDateInsideRange(const MetaData& track, const int min, const int max)
	{
		// date = today - 100
		// minDate = today - 120
		// maxDate = today - 50

		const auto date = Util::intToDate(track.createdDate()).date();
		const auto today = QDate::currentDate();

		const auto minDate = today.addDays(-max);  // today - 14
		const auto maxDate = today.addDays(-min);  // today - 120

		return ((date >= minDate) && (date <= maxDate)) ||
		       ((date <= minDate) && (date >= maxDate));
	}

	QString oldString(const QString& value)
	{
		return QObject::tr("%1 old").arg(value);
	}
}

SmartPlaylistByRelativeDate::SmartPlaylistByRelativeDate(const int id, const int value1, const int value2,
                                                         const bool isRandomized, const LibraryId libraryId) :
	SmartPlaylist {id, {value1, value2}, isRandomized, libraryId} {}

SmartPlaylistByRelativeDate::~SmartPlaylistByRelativeDate() = default;

int SmartPlaylistByRelativeDate::minimumValue() const { return 0; }

int SmartPlaylistByRelativeDate::maximumValue() const { return MaxYears * DaysPerYear; }

MetaDataList SmartPlaylistByRelativeDate::filterTracks(MetaDataList tracks)
{
	tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [&](const auto& track) {
		return !isDateInsideRange(track, value(0), value(1));
	}), tracks.end());

	return tracks;
}

QString SmartPlaylistByRelativeDate::classType() const { return SmartPlaylistByRelativeDate::ClassType; }

QString SmartPlaylistByRelativeDate::displayClassType() const
{
	return QObject::tr("Age of tracks");
}

QString SmartPlaylistByRelativeDate::name() const
{
	const auto from = std::min(value(0), value(1));
	const auto to = std::max(value(0), value(1));
	const auto sc = stringConverter();
	const auto oldFromString = oldString(sc->intToUserString(from));
	const auto oldToString = oldString(sc->intToUserString(to));

	if(from == to)
	{
		return oldFromString;
	}

	if(from == 0)
	{
		return QString("≤ %1")
			.arg(oldToString);
	}

	if(to == maximumValue())
	{
		return QString("≥ %1")
			.arg(oldFromString);
	}

	return QString("%1 - %2")
		.arg(sc->intToUserString(from))
		.arg(oldToString);
}

SmartPlaylists::Type SmartPlaylistByRelativeDate::type() const { return SmartPlaylists::Type::CreatedRelative; }

SmartPlaylists::StringConverterPtr SmartPlaylistByRelativeDate::createConverter() const
{
	return std::make_shared<SmartPlaylists::TimeSpanConverter>();
}

SmartPlaylists::InputFormat SmartPlaylistByRelativeDate::inputFormat() const
{
	return SmartPlaylists::InputFormat::TimeSpan;
}
