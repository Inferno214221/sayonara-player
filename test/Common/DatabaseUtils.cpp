/* DatabaseUtils.cpp, (Created on 01.01.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of Sayonara Player
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
#include "DatabaseUtils.h"
#include "Database/Module.h"

#include <QSqlQuery>

namespace Test::DB
{
	void deleteAllAlbums(::DB::Module* db)
	{
		db->runQuery("DELETE FROM albums;", "Could not delete all albums");
	}

	void deleteAllArtists(::DB::Module* db)
	{
		db->runQuery("DELETE FROM artists;", "Could not delete all artists");

	}

	void deleteAllTracks(::DB::Module* db)
	{
		db->runQuery("DELETE FROM tracks;", "Could not delete all tracks");
	}

	void deleteAllTracks(::DB::Module* db, const LibraryId libraryId)
	{
		db->runQuery("DELETE FROM tracks WHERE libraryId=:libraryId;",
		             {":libraryId", static_cast<int>(libraryId)},
		             "Could not delete all tracks");
	}

	void clearDatabase(::DB::Module* db)
	{
		deleteAllTracks(db);
		deleteAllAlbums(db);
		deleteAllArtists(db);
	}
} // Test