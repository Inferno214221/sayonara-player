/* LibraryDatabase.h */

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

#ifndef LIBRARYDATABASE_H
#define LIBRARYDATABASE_H

#include "Database/Base.h"
#include "Database/Albums.h"
#include "Database/Artists.h"
#include "Database/Tracks.h"
#include "Database/Library.h"
#include "Utils/Pimpl.h"

namespace DB
{
	class LibraryDatabase :
			public DB::Albums,
			public DB::Artists,
			public DB::Tracks,
			public DB::SearchableModule
	{
		PIMPL(LibraryDatabase)

		public:
			enum class ArtistIDField : uint8_t
			{
				AlbumArtistID,
				ArtistID
			};

			LibraryDatabase(const QString& connection_name, DbId db_id, LibraryId library_id);
			virtual ~LibraryDatabase() override;

			void change_artistid_field(ArtistIDField field);

			void clear();
			virtual bool store_metadata(const MetaDataList& v_md);

			LibraryId library_id() const override;
			QString artistid_field() const override;
			QString artistname_field() const override;
			QString track_view() const override;
			QString track_search_view() const override;

			::Library::SearchModeMask search_mode() const override;
			void update_search_mode(::Library::SearchModeMask smm) override;

			Module* module() override;
			const Module* module() const override;
	};
}

#endif // LIBRARYDATABASE_H
