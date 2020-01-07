/* Sorting.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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
	return
		(so.so_albums == so_albums) &&
		(so.so_artists == so_artists) &&
		(so.so_tracks == so_tracks);
}


QString Library::Sortings::toString() const
{
	return
		QString("%1,%2,%3")
			.arg(int(so_albums))
			.arg(int(so_artists))
			.arg(int(so_tracks));
}


bool Library::Sortings::loadFromString(const QString& str)
{
	QStringList lst = str.split(",");
	if(lst.size() < 3){
		return false;
	}

	this->so_albums = static_cast<Library::SortOrder>(lst[0].toInt());
	this->so_artists = static_cast<Library::SortOrder>(lst[1].toInt());
	this->so_tracks = static_cast<Library::SortOrder>(lst[2].toInt());

	return true;

}
