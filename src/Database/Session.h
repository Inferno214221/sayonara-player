/* Session.h */

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



#ifndef DB_SESSION_H
#define DB_SESSION_H

#include "Database/Module.h"
#include <QStringList>
#include <QDateTime>

class MetaData;
class MetaDataList;

namespace DB
{
	class Session :
			private DB::Module
	{

		public:
			Session(const QString& connection_name, DbId db_id);
			~Session();

			PairList<uint64_t, TrackID> get_sessions(uint64_t beginning);

			bool add_track(const QString& session_id, uint64_t current_date_time, const MetaData& md);
			void clear();
	};
}

#endif // DB_SESSION_H
