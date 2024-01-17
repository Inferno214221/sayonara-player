/* Sorting.h */

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

#ifndef LIBRARY_SORTING_H
#define LIBRARY_SORTING_H

#include "Sortorder.h"
#include "Utils/Settings/SettingConvertible.h"

class QString;
namespace Library
{
	struct Sortings :
		public SettingConvertible
	{
		Sortings() = default;
		~Sortings() override = default;
		Sortings(const Sortings& other);
		Sortings& operator=(const Sortings& other);
		bool operator==(const Sortings& other) const;

		// SettingConvertible
		[[nodiscard]] QString toString() const override;
		bool loadFromString(const QString& str) override;

		AlbumSortorder album {AlbumSortorder::NameAsc};
		ArtistSortorder artist {ArtistSortorder::NameAsc};
		TrackSortorder tracks {TrackSortorder::AlbumAsc};
	};
}

#endif // SORTING_H
