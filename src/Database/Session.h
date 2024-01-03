/* Session.h */

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
			Session(const QString& connectionName, DbId databaseId);
			~Session() override;

			[[nodiscard]] ::Session::EntryListMap
			getSessions(const QDateTime& dateTimeBegin, const QDateTime& dateTimeEnd);
			[[nodiscard]] QList<::Session::Id> getSessionKeys();

			[[nodiscard]] ::Session::Id createNewSession() const;
			bool addTrack(::Session::Id sessionId, const MetaData& track, const QDateTime& dateTime);

			bool clear();
			bool clearBefore(const QDateTime& datetime);
	};
}

#endif // DB_SESSION_H
