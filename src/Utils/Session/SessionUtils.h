/* SessionUtils.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef SESSIONTYPES_H
#define SESSIONTYPES_H

#include <QMap>
#include <QVector>

#include "Utils/typedefs.h"
#include "Utils/MetaData/MetaData.h"

namespace Session
{
	using Timecode = uint64_t;
	using Id = Timecode;

	struct Entry
	{
		Session::Id sessionId;
		Timecode timecode;
		MetaData track;

		bool operator==(const Entry& other) const;
	};

	using EntryList = QVector<Entry>;
	using EntryListMap = QMap<Session::Id, EntryList>;

	Timecode dayBegin(Session::Id id);
	Timecode dayEnd(Session::Id id);
	Timecode now();
}

Q_DECLARE_METATYPE(Session::Id)

#endif // SESSIONTYPES_H
