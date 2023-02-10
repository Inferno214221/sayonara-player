/* DBusHandler.cpp */

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

#include "DBusHandler.h"
#include "DBus/DBusMPRIS.h"
#include "DBus/DBusMediaKeysInterfaceMate.h"
#include "DBus/DBusMediaKeysInterfaceGnome.h"
#include "DBus/DBusNotifications.h"

#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Logger/Logger.h"
#include "Interfaces/PlaylistInterface.h"

#include <QMainWindow>
#include <QDBusConnectionInterface>
#include <QList>

struct DBusHandler::Private
{
	DBusMPRIS::MediaPlayer2* dbusMpris;
	DBusMediaKeysInterfaceMate* dbusMate;
	DBusMediaKeysInterfaceGnome* dbusGnome;
	DBusNotifications* dbusNotifications;

	Private(QMainWindow* mainWindow, PlayManager* playManager, PlaylistAccessor* playlistAccessor,
	        NotificationHandler* notificationHandler, DBusHandler* parent) :
		dbusMpris {new DBusMPRIS::MediaPlayer2(mainWindow, playManager, playlistAccessor, parent)},
		dbusMate {new DBusMediaKeysInterfaceMate(playManager, parent)},
		dbusGnome {new DBusMediaKeysInterfaceGnome(playManager, parent)},
		dbusNotifications {new DBusNotifications(notificationHandler, parent)} {}
};

DBusHandler::DBusHandler(QMainWindow* mainWindow, PlayManager* playManager, PlaylistAccessor* playlistAccessor,
                         NotificationHandler* notificationHandler, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(mainWindow, playManager, playlistAccessor, notificationHandler, this);

	auto* dbusInterface = QDBusConnection::sessionBus().interface();
	if(dbusInterface)
	{
		connect(dbusInterface, &QDBusConnectionInterface::serviceRegistered,
		        this, &DBusHandler::serviceRegistered);
		connect(dbusInterface, &QDBusConnectionInterface::serviceUnregistered,
		        this, &DBusHandler::serviceUnregistered);
	}
}

DBusHandler::~DBusHandler() = default;

void DBusHandler::serviceRegistered(const QString& service_name)
{
	spLog(Log::Info, this) << "Service " << service_name << " registered";
}

void DBusHandler::serviceUnregistered(const QString& service_name)
{
	spLog(Log::Warning, this) << "Service " << service_name << " unregistered";
}
