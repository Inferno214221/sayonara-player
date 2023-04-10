/* Session.h */

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

#ifndef SAYONARA_PLAYER_SESSION_H
#define SAYONARA_PLAYER_SESSION_H

#include "Utils/Pimpl.h"
#include "Utils/Session/SessionUtils.h"

#include <QObject>
#include <QDateTime>

class PlayManager;
class QDateTime;

namespace Session
{
	class Manager :
		public QObject
	{
		Q_OBJECT
		PIMPL(Manager)

		signals:
			void sigSessionChanged(Session::Id id);
			void sigSessionDeleted(Session::Id id);

		public:
			explicit Manager(PlayManager* playManager);
			~Manager() override;

			EntryListMap history(const QDateTime& begin, const QDateTime& end);
			EntryListMap historyForDay(const QDateTime& dt);
			EntryListMap historyEntries(int dayIndex, int count);
			[[nodiscard]] bool isEmpty() const;
			void clearAllHistory();
			void clearAllHistoryBefore(const QDateTime& dt);

		private slots:
			void positionChanged(MilliSeconds ms);
	};
}

#endif // SAYONARA_PLAYER_SESSION_H
