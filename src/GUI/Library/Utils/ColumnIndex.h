/* ColumnIndex.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

namespace ColumnIndex
{
	enum class Album : quint8
	{
		MultiDisc=0,
		Name=1,
		Duration=2,
		NumSongs=3,
		Year=4,
		Rating=5
	};

	enum class Artist : quint8
	{
		NumAlbums=0,
		Name=1,
		Tracks=2
	};

	enum class Track : quint8
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
		Rating
	};
}

#endif // LIBRARYVIEWCOLUMNS_H
