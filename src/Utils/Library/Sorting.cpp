/* Sorting.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

Library::Sortings::Sortings()
{
	so_artists = Library::SortOrder::ArtistNameAsc;
	so_albums = Library::SortOrder::AlbumNameAsc;
	so_tracks = Library::SortOrder::TrackAlbumAsc;
}

Library::Sortings::Sortings(const Sortings& other) :
	CASSIGN(so_albums),
	CASSIGN(so_artists),
	CASSIGN(so_tracks)
{}

Library::Sortings::~Sortings() {}

Library::Sortings& Library::Sortings::operator=(const Library::Sortings& other)
{
	ASSIGN(so_albums);
	ASSIGN(so_artists);
	ASSIGN(so_tracks);

	return (*this);
}

bool Library::Sortings::operator==(Library::Sortings so)
{
	return  (so.so_albums == so_albums) &&
		(so.so_artists == so_artists) &&
		(so.so_tracks == so_tracks);
}


QString Library::Sortings::toString() const
{
	return QString::number((int) so_albums) + "," +
		QString::number((int) so_artists) + "," +
		QString::number((int) so_tracks);
}


Library::Sortings Library::Sortings::fromString(const QString& str)
{
	Library::Sortings so;
	QStringList lst = str.split(",");
		so.so_albums = (Library::SortOrder) lst[0].toInt();
		so.so_artists = (Library::SortOrder) lst[1].toInt();
		so.so_tracks = (Library::SortOrder) lst[2].toInt();
	return so;
}
