/* ColumnIndex.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#ifndef LIBRARYVIEWCOLUMNS_H
#define LIBRARYVIEWCOLUMNS_H

#define COL_ALBUM_MACROS
#define COL_ARTIST_MACROS

#include <cstdint>
class QString;

namespace ColumnIndex
{
	using IntegerType=uint8_t;

	enum class Album : IntegerType
	{
		MultiDisc=0,
		Name,
		Duration,
		AlbumArtist,
		NumSongs,
		Year,
		Rating
	};

	enum class Artist : IntegerType
	{
		//Undefined=0,
		Name=0,
		Tracks
	};

	enum class Track : IntegerType
	{
		TrackNumber=0,
		Title,
		Artist,
		Album,
		Discnumber,
		Year,
		Length,
		Bitrate,
		Filesize,
		Filetype,
		AddedDate,
		ModifiedDate,
		Rating
	};
}

#endif // LIBRARYVIEWCOLUMNS_H
