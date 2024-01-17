/* Sortorder.h */

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

#ifndef SORTORDER_H_
#define SORTORDER_H_

namespace Library
{
	enum class SortOrder :
		unsigned char
	{
		NoSorting = 0,
		ArtistNameAsc,
		ArtistNameDesc,
		ArtistTrackcountAsc,
		ArtistTrackcountDesc,
		AlbumNameAsc,
		AlbumNameDesc,
		AlbumYearAsc,
		AlbumYearDesc,
		AlbumTracksAsc,
		AlbumTracksDesc,
		AlbumDurationAsc,
		AlbumDurationDesc,
		AlbumRatingAsc,
		AlbumRatingDesc,
		AlbumCreationDateAsc,
		AlbumCreationDateDesc,
		TrackNumAsc,
		TrackNumDesc,
		TrackTitleAsc,
		TrackTitleDesc,
		TrackAlbumAsc,
		TrackAlbumDesc,
		TrackArtistAsc,
		TrackArtistDesc,
		TrackAlbumArtistAsc,
		TrackAlbumArtistDesc,
		TrackYearAsc,
		TrackYearDesc,
		TrackLenghtAsc,
		TrackLengthDesc,
		TrackBitrateAsc,
		TrackBitrateDesc,
		TrackSizeAsc,
		TrackSizeDesc,
		TrackDiscnumberAsc,
		TrackDiscnumberDesc,
		TrackRatingAsc,
		TrackRatingDesc,
		TrackFilenameAsc,
		TrackFilenameDesc,
		TrackFiletypeAsc,
		TrackFiletypeDesc,
		TrackDateModifiedAsc,
		TrackDateModifiedDesc,
		TrackDateAddedAsc,
		TrackDateAddedDesc
	};
}

#endif
