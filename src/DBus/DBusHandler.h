/* DBusHandler.h */

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

#ifndef DBUSHANDLER_H
#define DBUSHANDLER_H

#include "Utils/Pimpl.h"
#include <QObject>

class QMainWindow;
class PlayManager;
class PlaylistAccessor;

namespace DBusMPRIS
{
	class MediaPlayer2;
}

class NotificationHandler;
class DBusHandler :
	public QObject
{
	Q_OBJECT
	PIMPL(DBusHandler)

	public:
		DBusHandler(QMainWindow* mainWindow, PlayManager* playManager, PlaylistAccessor* playlistAccessor,
		            NotificationHandler* notificationHandler, QObject* parent = nullptr);
		virtual ~DBusHandler();

	private slots:
		void serviceRegistered(const QString& serviceName);
		void serviceUnregistered(const QString& serviceName);
};

#endif // DBUSHANDLER_H
