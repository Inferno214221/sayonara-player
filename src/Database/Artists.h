/* DatabaseArtists.h */

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

#ifndef DATABASEARTISTS_H
#define DATABASEARTISTS_H

#include "Database/Utils.h"
#include "Utils/typedefs.h"

namespace Library
{
	class Filter;
}

class Artist;
class ArtistList;
class QSqlQuery;

namespace DB
{
	class Module;
	class Artists
	{
		public:
			Artists();
			virtual ~Artists();

			virtual bool dbFetchArtists(QSqlQuery& q, ArtistList& result) const;

			[[nodiscard]] virtual ArtistId getArtistID(const QString& artist) const;
			virtual bool getArtistByID(ArtistId id, Artist& artist) const;
			virtual bool getArtistByID(ArtistId id, Artist& artist, bool alsoEmpty) const;

			virtual bool getAllArtists(ArtistList& result, bool alsoEmpty) const;
			virtual bool getAllArtistsBySearchString(const ::Library::Filter& filter, ArtistList& result) const;

			virtual bool deleteArtist(ArtistId id);

			virtual ArtistId insertArtistIntoDatabase(const QString& artist);
			[[maybe_unused]] virtual ArtistId insertArtistIntoDatabase(const Artist& artist);

		protected:
			[[nodiscard]] virtual ArtistIdInfo artistIdInfo() const = 0;
			[[nodiscard]] virtual QString trackView() const = 0;
			[[nodiscard]] virtual QString trackSearchView() const = 0;

			virtual Module* module() = 0;
			[[nodiscard]] virtual const Module* module() const = 0;

			virtual void updateArtistCissearch();
			virtual void deleteAllArtists();

		private:
			[[nodiscard]] virtual QString fetchQueryArtists(bool also_empty) const;
	};
}

#endif // DATABASEARTISTS_H
