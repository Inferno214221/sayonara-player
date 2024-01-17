/* Sorting.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "Sorting.h"
#include "Utils/Pimpl.h" // CASSIGN
#include <QStringList>

namespace Library
{
	namespace
	{
		template<typename T>
		T applyValue(const int val, const T defaultValue)
		{
			const auto isValid = (val >= static_cast<int>(T::NoSorting)) &&
			                     (val < static_cast<int>(T::Last));

			return isValid ? static_cast<T>(val) : defaultValue;
		}
	}

	QString Sortings::toString() const
	{
		return
			QString("%1,%2,%3")
				.arg(static_cast<int>(album))
				.arg(static_cast<int>(artist))
				.arg(static_cast<int>(tracks));
	}

	bool Sortings::loadFromString(const QString& str)
	{
		const auto lst = str.split(",");
		if(lst.size() < 3)
		{
			return false;
		}

		album = applyValue<AlbumSortorder>(lst[0].toInt(), Sortings().album);
		artist = applyValue<ArtistSortorder>(lst[1].toInt(), Sortings().artist);
		tracks = applyValue<TrackSortorder>(lst[2].toInt(), Sortings().tracks);

		return true;
	}

	bool Sortings::operator==(const Sortings& other) const
	{
		return (album == other.album) &&
		       (artist == other.artist) &&
		       (tracks == other.tracks);
	}

	Sortings& Sortings::operator=(const Sortings& other)
	{
		album = other.album;
		artist = other.artist;
		tracks = other.tracks;
		return *this;
	}

	Sortings::Sortings(const Sortings& other) :
		album {other.album},
		artist {other.artist},
		tracks {other.tracks} {}
}