/* DatabaseArtists.h */

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

#ifndef DATABASEARTISTS_H
#define DATABASEARTISTS_H

#include "Database/DatabaseSearchMode.h"
#include "Database/DatabaseModule.h"
#include "Utils/Library/Sortorder.h"

namespace Library {class Filter;}

class Artist;
class ArtistList;

namespace DB
{
	class Artists :
			private SearchMode
	{
		PIMPL(Artists)

		public:

			Artists(const QString& connection_name, DbId db_id, LibraryId library_id);
			~Artists();

			virtual bool db_fetch_artists(Query& q, ArtistList& result);

			virtual ArtistId getArtistID (const QString& artist);
			virtual bool getArtistByID(int id, Artist& artist, bool also_empty=false);

			virtual bool getAllArtists(ArtistList& result, bool also_empty);
			virtual bool getAllArtists(ArtistList& result, ::Library::SortOrder sortorder = ::Library::SortOrder::ArtistNameAsc, bool also_empty=false);
			virtual bool getAllArtistsBySearchString(const ::Library::Filter& filter, ArtistList& result, ::Library::SortOrder sortorder = ::Library::SortOrder::ArtistNameAsc);

			virtual ArtistId insertArtistIntoDatabase(const QString& artist);
			virtual ArtistId insertArtistIntoDatabase(const Artist& artist);
			virtual ArtistId updateArtist(const Artist& artist);

			virtual void updateArtistCissearch();


		private:
			QString _create_order_string(::Library::SortOrder sort);

		protected:
			void change_artistid_field(const QString& id, const QString& name);
			void change_track_lookup_field(const QString& track_lookup_field);

			virtual QString fetch_query_artists(bool also_empty=false) const;

	};
}

#endif // DATABASEARTISTS_H
