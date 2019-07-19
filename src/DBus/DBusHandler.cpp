/* DBusHandler.cpp */

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
	DBusMPRIS::MediaPlayer2*		dbus_mpris=nullptr;
	DBusMediaKeysInterfaceMate*		dbus_mate=nullptr;
	DBusMediaKeysInterfaceGnome*	dbus_gnome=nullptr;
	DBusNotifications*				dbus_notifications=nullptr;

	Private(QMainWindow* main_window, DBusHandler* parent)
	{
		dbus_mpris	= new DBusMPRIS::MediaPlayer2(main_window, parent);
		dbus_mate = new DBusMediaKeysInterfaceMate(parent);
		dbus_gnome = new DBusMediaKeysInterfaceGnome(parent);
		dbus_notifications = new DBusNotifications(parent);
	}
};

DBusHandler::DBusHandler(QMainWindow* main_window, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(main_window, this);

	QDBusConnectionInterface* dbus_interface = QDBusConnection::sessionBus().interface();
	if(dbus_interface)
	{
		connect(dbus_interface, &QDBusConnectionInterface::serviceRegistered,
				this, &DBusHandler::service_registered);
		connect(dbus_interface, &QDBusConnectionInterface::serviceUnregistered,
				this, &DBusHandler::service_unregistered);
	}
}

DBusHandler::~DBusHandler() {}

void DBusHandler::service_registered(const QString& service_name)
{
	sp_log(Log::Info, this) << "Service " << service_name << " registered";
}

void DBusHandler::service_unregistered(const QString& service_name)
{
	sp_log(Log::Warning, this) << "Service " << service_name << " unregistered";
}
