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

#include <variant>

namespace Library
{
	enum class ArtistSortorder :
		unsigned char
	{
		NoSorting = 0,
		NameAsc,
		NameDesc,
		TrackcountAsc,
		TrackcountDesc,
		Last
	};

	enum class AlbumSortorder :
		unsigned char
	{
		NoSorting = 0,
		NameAsc,
		NameDesc,
		YearAsc,
		YearDesc,
		TracksAsc,
		TracksDesc,
		DurationAsc,
		DurationDesc,
		RatingAsc,
		RatingDesc,
		CreationDateAsc,
		CreationDateDesc,
		AlbumArtistAsc,
		AlbumArtistDesc,
		Last
	};

	enum class TrackSortorder :
		unsigned char
	{
		NoSorting = 0,
		TrackNumberAsc,
		TrackNumberDesc,
		TitleAsc,
		TitleDesc,
		ArtistAsc,
		ArtistDesc,
		AlbumAsc,
		AlbumDesc,
		AlbumArtistAsc,
		AlbumArtistDesc,
		YearAsc,
		YearDesc,
		LengthAsc,
		LengthDesc,
		BitrateAsc,
		BitrateDesc,
		SizeAsc,
		SizeDesc,
		DiscnumberAsc,
		DiscnumberDesc,
		RatingAsc,
		RatingDesc,
		FilenameAsc,
		FilenameDesc,
		FiletypeAsc,
		FiletypeDesc,
		DateModifiedAsc,
		DateModifiedDesc,
		DateAddedAsc,
		DateAddedDesc,
		Last
	};

	using VariableSortorder = std::variant<AlbumSortorder, ArtistSortorder, TrackSortorder>;
}

#endif
