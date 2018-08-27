/* DatabaseAlbums.h */

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

#ifndef DATABASEALBUMS_H
#define DATABASEALBUMS_H

#include "Database/SearchableModule.h"
#include "Utils/Library/Sortorder.h"

namespace Library {class Filter;}
class Album;
class AlbumList;

namespace DB
{
	class Albums :
			private SearchableModule
	{
		PIMPL(Albums)

		public:
			Albums(const QString& connection_name, DbId db_id, LibraryId library_id);
			virtual ~Albums();

			virtual bool db_fetch_albums(Query& q, AlbumList& result);

			virtual AlbumId getAlbumID (const QString& album);

			virtual bool getAlbumByID(AlbumId id, Album& album, bool also_empty=false);

			virtual bool getAllAlbums(AlbumList& result, bool also_empty);
			virtual bool getAllAlbums(AlbumList& result, ::Library::SortOrder sortorder=::Library::SortOrder::AlbumNameAsc, bool also_empty=false);


			virtual bool getAllAlbumsByArtist(ArtistId artist, AlbumList& result);
			virtual bool getAllAlbumsByArtist(ArtistId artist, AlbumList& result, const ::Library::Filter& filter, ::Library::SortOrder sortorder = ::Library::SortOrder::AlbumNameAsc);
			virtual bool getAllAlbumsByArtist(IdList artists, AlbumList& result);
			virtual bool getAllAlbumsByArtist(IdList artists, AlbumList& result, const ::Library::Filter& filter, ::Library::SortOrder sortorder = ::Library::SortOrder::AlbumNameAsc);

			virtual bool getAllAlbumsBySearchString(const ::Library::Filter& filter, AlbumList& result, ::Library::SortOrder sortorder = ::Library::SortOrder::AlbumNameAsc);

			virtual AlbumId insertAlbumIntoDatabase (const QString & album);
			virtual AlbumId insertAlbumIntoDatabase (const Album& album);

			virtual AlbumId updateAlbum(const Album& album);

			virtual void updateAlbumCissearch();

		protected:
			virtual QString artistid_field() const=0;

		private:
			virtual QString fetch_query_albums(bool also_empty=false) const;
	};
}

#endif // DATABASEALBUMS_H
