/* LastFM.h */

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


/*
 * LastFM.h
 *
 *  Created on: Apr 19, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef SAYONARA_LASTFM_H
#define SAYONARA_LASTFM_H

#include "Utils/Pimpl.h"

#include <QObject>

class PlayManager;
class NotificationHandler;

namespace LastFM
{
	class Base :
		public QObject
	{
		Q_OBJECT
		PIMPL(Base)

		signals:
			void sigLoggedIn(bool);

		public:
			Base(PlayManager* playManager, NotificationHandler* notificationHandler);
			~Base() override;

			void login(const QString& username, const QString& password);
			bool isLoggedIn();

		private slots:
			void activeChanged();
			void loginThreadFinished(bool success);
			void currentTrackChanged(const MetaData& track);

			void scrobble();
			void scrobbleErrorReceived(const QString& str);

			void trackChangedTimerTimedOut();
	};
}

#endif /* SAYONARA_LASTFM_H */
