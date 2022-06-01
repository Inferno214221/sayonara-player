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

SmartPlaylistByRating::SmartPlaylistByRating(const int id, const int ratingFrom, const int ratingTo) :
	SmartPlaylist(id, ratingFrom, ratingTo) {}

SmartPlaylistByRating::~SmartPlaylistByRating() = default;

MetaDataList SmartPlaylistByRating::filterTracks(MetaDataList tracks)
{
	const auto minimumRating = static_cast<Rating>(from());
	const auto maximumRating = static_cast<Rating>(to());

	tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [&](const auto& track) {
		return (track.rating() < minimumRating) || (track.rating() > maximumRating);
	}), tracks.end());

	return tracks;
}

QString SmartPlaylistByRating::name() const
{
	if(from() == to())
	{
		return QObject::tr("%1 stars")
			.arg(from());
	}

	if((from() < maximumValue()) && (to() == maximumValue()))
	{
		return QObject::tr("At least %1 stars")
			.arg(from());
	}

	if(from() == minimumValue() && (to() > minimumValue()))
	{
		return QObject::tr("At most %1 stars")
			.arg(to());
	}

	return QObject::tr("Between %1 and %2 stars")
		.arg(from())
		.arg(to());
}

QString SmartPlaylistByRating::classType() const { return SmartPlaylistByRating::ClassType; }

QString SmartPlaylistByRating::displayClassType() const { return Lang::get(Lang::Rating); }

int SmartPlaylistByRating::minimumValue() const { return static_cast<int>(Rating::Zero); }

int SmartPlaylistByRating::maximumValue() const { return static_cast<int>(Rating::Five); }

SmartPlaylists::Type SmartPlaylistByRating::type() const { return SmartPlaylists::Type::Rating; }
