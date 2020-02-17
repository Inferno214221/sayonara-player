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

#include <QMainWindow>
#include <QDBusConnectionInterface>
#include <QList>

struct DBusHandler::Private
{
	DBusMPRIS::MediaPlayer2*		dbusMpris=nullptr;
	DBusMediaKeysInterfaceMate*		dbusMate=nullptr;
	DBusMediaKeysInterfaceGnome*	dbusGnome=nullptr;
	DBusNotifications*				dbusNotifications=nullptr;

	Private(QMainWindow* mainWindow, DBusHandler* parent)
	{
		dbusMpris	= new DBusMPRIS::MediaPlayer2(mainWindow, parent);
		dbusMate = new DBusMediaKeysInterfaceMate(parent);
		dbusGnome = new DBusMediaKeysInterfaceGnome(parent);
		dbusNotifications = new DBusNotifications(parent);
	}
};

DBusHandler::DBusHandler(QMainWindow* mainWindow, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(mainWindow, this);

	QDBusConnectionInterface* dbus_interface = QDBusConnection::sessionBus().interface();
	if(dbus_interface)
	{
		connect(dbus_interface, &QDBusConnectionInterface::serviceRegistered,
				this, &DBusHandler::serviceRegistered);
		connect(dbus_interface, &QDBusConnectionInterface::serviceUnregistered,
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
