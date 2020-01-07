/* DatabaseLibrary.h */

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

#ifndef DATABASELIBRARY_H
#define DATABASELIBRARY_H

#include "Database/Module.h"

#include <QList>
#include <QMap>

class MetaDataList;

namespace Library
{
	class Info;
}

namespace DB
{
	class Library :
			private Module
	{
		PIMPL(Library)

		public:
			Library(const QString& connection_name, DbId db_id);
			~Library() override;

			using LibraryInfo=::Library::Info;
			QList<LibraryInfo> get_all_libraries();

			bool insert_library(LibraryId library_id, const QString& library_name, const QString& library_path, int index);
			bool edit_library(LibraryId library_id, const QString& new_name, const QString& new_path);
			bool remove_library(LibraryId library_id);
			bool reorder_libraries(const QMap<LibraryId, int>& order);

			virtual void drop_indexes();
			virtual void create_indexes();

			virtual void add_album_artists();
	};
}

#endif // DATABASELIBRARY_H
