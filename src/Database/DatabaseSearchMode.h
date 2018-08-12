/* DatabaseSearchMode.h */

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

#ifndef DATABASESEARCHMODE_H
#define DATABASESEARCHMODE_H

#include "Utils/Pimpl.h"
#include "Utils/Library/SearchMode.h"
#include "Database/DatabaseModule.h"

class QSqlDatabase;

namespace DB
{
	class SearchMode :
		public Module
	{
		PIMPL(SearchMode)

		private:
			void init();

		protected:
			SearchMode(const QString& connection_name, DbId db_id);

		public:
			virtual ~SearchMode();

			::Library::SearchModeMask search_mode();
			void update_search_mode();
		};
}

#endif // DATABASESEARCHMODE_H
