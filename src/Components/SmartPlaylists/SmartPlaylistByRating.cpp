/* SmartPlaylistByRating.cpp */
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
#include "SmartPlaylistByRating.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QObject>

namespace
{
	QString starsTranslation(const int value)
	{
		return QObject::tr("%1 star(s)", "", value);
	}
}

SmartPlaylistByRating::SmartPlaylistByRating(const int id, const int ratingFrom, const int ratingTo,
                                             const bool isRandomized) :
	SmartPlaylist(id, {ratingFrom, ratingTo}, isRandomized) {}

SmartPlaylistByRating::~SmartPlaylistByRating() = default;

MetaDataList SmartPlaylistByRating::filterTracks(MetaDataList tracks)
{
	const auto minimumRating = static_cast<Rating>(std::min(value(0), value(1)));
	const auto maximumRating = static_cast<Rating>(std::max(value(0), value(1)));

	tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [&](const auto& track) {
		return (track.rating() < minimumRating) || (track.rating() > maximumRating);
	}), tracks.end());

	return tracks;
}

QString SmartPlaylistByRating::name() const
{
	const auto from = std::min(value(0), value(1));
	const auto to = std::max(value(0), value(1));
	const auto nStarsFrom = starsTranslation(from);
	const auto nStarsTo = starsTranslation(to);

	if(from == to)
	{
		return nStarsFrom;
	}

	if((from < maximumValue()) && (to == maximumValue()))
	{
		return QString("≥ %1").arg(nStarsFrom);
	}

	if(from == minimumValue() && (to > minimumValue()))
	{
		return QString("≤ %1").arg(nStarsTo);
	}

	return QString("%1 - %2")
		.arg(from)
		.arg(nStarsTo);
}

QString SmartPlaylistByRating::classType() const { return SmartPlaylistByRating::ClassType; }

QString SmartPlaylistByRating::displayClassType() const { return Lang::get(Lang::Rating); }

int SmartPlaylistByRating::minimumValue() const { return static_cast<int>(Rating::Zero); }

int SmartPlaylistByRating::maximumValue() const { return static_cast<int>(Rating::Five); }

SmartPlaylists::Type SmartPlaylistByRating::type() const { return SmartPlaylists::Type::Rating; }
