/* MetaDataSorting.h */

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

#ifndef METADATASORTING_H
#define METADATASORTING_H

#include "Utils/Library/Sortorder.h"
#include "Utils/globals.h"
#include <cstdint>
#include <type_traits>

class MetaDataList;
class AlbumList;
class ArtistList;

namespace MetaDataSorting
{
	enum class SortMode :
		uint16_t
	{
		None = 0,
		CaseInsensitive = (1 << 0),
		IgnoreSpecialChars = (1 << 1),
		IgnoreDiacryticChars = (1 << 2),
		IgnoreArticle = (1 << 3)
	};

	using SortModeMask = std::underlying_type_t<SortMode>;

	void sortMetadata(MetaDataList& tracks, Library::TrackSortorder sortOrder, SortModeMask sortMode = +SortMode::None);
	void sortAlbums(AlbumList& albums, Library::AlbumSortorder sortOrder, SortModeMask sortMode = +SortMode::None);
	void sortArtists(ArtistList& artists, Library::ArtistSortorder sortOrder, SortModeMask sortMode = +SortMode::None);
}

#endif // METADATASORTING_H
