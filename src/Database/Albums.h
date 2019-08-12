/* DatabaseAlbums.h */

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

#ifndef DATABASEALBUMS_H
#define DATABASEALBUMS_H

#include "Database/SearchableModule.h"

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
			~Albums();

			virtual bool db_fetch_albums(Query& q, AlbumList& result) const;

			virtual AlbumId getAlbumID (const QString& album) const;

            virtual bool getAlbumByID(AlbumId id, Album& album) const;
			virtual bool getAlbumByID(AlbumId id, Album& album, bool also_empty) const;

			virtual bool getAllAlbums(AlbumList& result, bool also_empty) const;
			virtual bool getAllAlbumsByArtist(const IdList& artists, AlbumList& result, const ::Library::Filter& filter) const;

			virtual bool getAllAlbumsBySearchString(const ::Library::Filter& filter, AlbumList& result) const;

			virtual AlbumId insertAlbumIntoDatabase (const QString& album);
			virtual AlbumId insertAlbumIntoDatabase (const Album& album);

			virtual AlbumId updateAlbum(const Album& album);

			virtual void updateAlbumCissearch();

		protected:
			virtual QString artistid_field() const=0;

		private:
			virtual QString fetch_query_albums(bool also_empty) const;
	};
}

#endif // DATABASEALBUMS_H
