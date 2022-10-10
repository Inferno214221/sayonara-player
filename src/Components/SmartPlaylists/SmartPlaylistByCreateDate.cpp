/* SmartPlaylistByCreateDate.cpp */
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
#include "SmartPlaylistByCreateDate.h"
#include "DateConverter.h"

#include "Utils/Utils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language/Language.h"

#include <QDate>
#include <QLocale>

namespace
{
	bool isTrackDateInRange(const uint64_t trackDate, const int value1, const int value2)
	{
		const auto minimumValue = std::min(value1, value2);
		const auto maximumValue = std::max(value1, value2);
		const auto convertedTrackDate = static_cast<int>(trackDate / 1'000'000);

		return (convertedTrackDate >= minimumValue) && (convertedTrackDate <= maximumValue);
	}
}

SmartPlaylistByCreateDate::SmartPlaylistByCreateDate(const int id, const int value1, const int value2,
                                                     const bool isRandomized, const LibraryId libraryId) :
	SmartPlaylist(id, {value1, value2}, isRandomized, libraryId) {}

SmartPlaylistByCreateDate::~SmartPlaylistByCreateDate() = default;

int SmartPlaylistByCreateDate::minimumValue() const
{
	return SmartPlaylists::dateToInt(QDate(2000, 1, 1)); // NOLINT(readability-magic-numbers)
}

int SmartPlaylistByCreateDate::maximumValue() const
{
	return SmartPlaylists::dateToInt(QDate::currentDate());
}

MetaDataList SmartPlaylistByCreateDate::filterTracks(MetaDataList tracks)
{
	tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [&](const auto& track) {
		return !isTrackDateInRange(track.createdDate(), value(0), value(1));
	}), tracks.end());

	return tracks;
}

QString SmartPlaylistByCreateDate::classType() const { return SmartPlaylistByCreateDate::ClassType; }

QString SmartPlaylistByCreateDate::displayClassType() const { return Lang::get(Lang::Created); }

QString SmartPlaylistByCreateDate::name() const
{
	auto locale = QLocale();
	const auto minDate = SmartPlaylists::intToDate(value(0));
	const auto maxDate = SmartPlaylists::intToDate(value(1));

	const auto timeSpan = QObject::tr("%1 - %2")
		.arg(locale.toString(minDate, QLocale::ShortFormat))
		.arg(locale.toString(maxDate, QLocale::ShortFormat));

	return QString("%1: %2")
		.arg(Lang::get(Lang::Created))
		.arg(timeSpan);
}

SmartPlaylists::Type SmartPlaylistByCreateDate::type() const { return SmartPlaylists::Type::Created; }

SmartPlaylists::StringConverterPtr SmartPlaylistByCreateDate::createConverter() const
{
	return std::make_shared<SmartPlaylists::DateConverter>();
}

SmartPlaylists::InputFormat SmartPlaylistByCreateDate::inputFormat() const
{
	return SmartPlaylists::InputFormat::Calendar;
}
