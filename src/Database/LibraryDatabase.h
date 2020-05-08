/* LibraryDatabase.h */

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
			public DB::Module
	{
		PIMPL(LibraryDatabase)

		public:
			enum class ArtistIDField : uint8_t
			{
				AlbumArtistID,
				ArtistID
			};

			LibraryDatabase(const QString& connectionName, DbId databaseId, LibraryId libraryId);
			virtual ~LibraryDatabase() override;

			void changeArtistIdField(ArtistIDField field);

			void clear();
			virtual bool storeMetadata(const MetaDataList& tracks);

			LibraryId libraryId() const override;
			QString artistIdField() const override;
			QString artistNameField() const override;
			QString trackView() const override;
			QString trackSearchView() const override;

			void updateSearchMode();

			MetaDataList insertMissingArtistsAndAlbums(const MetaDataList& tracks);
			bool fixEmptyAlbums();

		protected:
			Module* module() override;
			const Module* module() const override;

			void initSearchMode();
	};
}

#endif // LIBRARYDATABASE_H
