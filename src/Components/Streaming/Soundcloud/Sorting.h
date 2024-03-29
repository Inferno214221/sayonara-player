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



#ifndef SORTING_H
#define SORTING_H

#include "Utils/Library/Sorting.h"

class ArtistList;
class AlbumList;
class MetaDataList;

namespace SC
{
	namespace Sorting
	{
		void sortArtists(ArtistList& artists, ::Library::ArtistSortorder so);
		void sortAlbums(AlbumList& albums, ::Library::AlbumSortorder so);
		void sortTracks(MetaDataList& tracks, ::Library::TrackSortorder so);
	}
}

#endif // SORTING_H
