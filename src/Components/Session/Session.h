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

#ifndef SESSION_H
#define SESSION_H

#include "Utils/Pimpl.h"
#include "Utils/Singleton.h"
#include "Utils/Session/SessionUtils.h"

#include <QObject>
#include <QDateTime>

class QDateTime;

namespace Session
{
	class Manager :
			public QObject
	{
		Q_OBJECT
		PIMPL(Manager)
		SINGLETON(Manager)

		signals:
			void sig_changed(Session::Id id);

		public:
			EntryListMap history(const QDateTime& dt_begin, const QDateTime& dt_end);
			EntryListMap history_for_day(const QDateTime& dt);
			EntryListMap history_entries(int day_index, int count);

		private slots:
			void position_changed(MilliSeconds ms);
	};
}

#endif // SESSION_H
