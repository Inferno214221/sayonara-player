/* Session.h */

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

#ifndef DB_SESSION_H
#define DB_SESSION_H

#include "Database/Module.h"
#include "Utils/Session/SessionUtils.h"

class MetaData;
class MetaDataList;
class QDateTime;

namespace DB
{
	class Session :
			private DB::Module
	{
		public:
			Session(const QString& connection_name, DbId db_id);
			~Session();

			::Session::EntryListMap get_sessions(const QDateTime& dt_begin, const QDateTime& dt_end);
			::Session::EntryList get_session(::Session::Id session_id);
			QList<::Session::Id> get_session_keys();

			::Session::Id create_new_session() const;
			bool add_track(::Session::Id session_id, const MetaData& md);

			bool clear();
			bool clear_before(const QDateTime& datetime);
	};
}

#endif // DB_SESSION_H
