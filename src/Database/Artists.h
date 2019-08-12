/* DatabaseArtists.h */

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

#ifndef DATABASEARTISTS_H
#define DATABASEARTISTS_H

#include "Database/SearchableModule.h"

namespace Library {class Filter;}

class Artist;
class ArtistList;

namespace DB
{
	class Artists :
			private SearchableModule
	{
		PIMPL(Artists)

		public:

			Artists(const QString& connection_name, DbId db_id, LibraryId library_id);
			~Artists() override;

			virtual bool db_fetch_artists(Query& q, ArtistList& result) const;

			virtual ArtistId getArtistID (const QString& artist) const;
            virtual bool getArtistByID(ArtistId id, Artist& artist) const;
			virtual bool getArtistByID(ArtistId id, Artist& artist, bool also_empty) const;

			virtual bool getAllArtists(ArtistList& result, bool also_empty) const;
			virtual bool getAllArtistsBySearchString(const ::Library::Filter& filter, ArtistList& result) const;

			virtual bool deleteArtist(ArtistId id);

			virtual ArtistId insertArtistIntoDatabase(const QString& artist);
			virtual ArtistId insertArtistIntoDatabase(const Artist& artist);

			virtual ArtistId updateArtist(const Artist& artist);

			virtual void updateArtistCissearch();

		protected:
			virtual QString artistid_field() const=0;
			virtual QString artistname_field() const=0;

		private:
			virtual QString fetch_query_artists(bool also_empty) const;
	};
}

#endif // DATABASEARTISTS_H
