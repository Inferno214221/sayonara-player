/* SessionUtils.cpp
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

#include "SessionUtils.h"
#include "Utils/Utils.h"

#include <QDateTime>

bool Session::Entry::operator==(const Session::Entry& other) const
{
	return
		(sessionId == other.sessionId) &&
		(timecode == other.timecode) &&
		(track.title() == other.track.title());
}

Session::Timecode Session::dayBegin(Session::Id id)
{
	QDateTime dt = Util::intToDate(id);
	dt.setTime(QTime(0, 0, 0));

	return Util::dateToInt(dt);
}

Session::Timecode Session::dayEnd(Session::Id id)
{
	QDateTime dt = Util::intToDate(id);
	dt.setTime(QTime(23, 59, 59));

	return Util::dateToInt(dt);
}

Session::Timecode Session::now()
{
	QDateTime dt = QDateTime::currentDateTime().toUTC();

	return Util::dateToInt(dt);
}
