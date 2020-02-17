/* DBusMediaKeysInterfaceGnome.cpp */

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

#include "DBusMediaKeysInterfaceGnome.h"
#include "DBus/gnome_settings_daemon.h"
#include "Utils/Logger/Logger.h"

struct DBusMediaKeysInterfaceGnome::Private
{
	OrgGnomeSettingsDaemonMediaKeysInterface* mediaKeyInterface=0;

	Private(DBusMediaKeysInterface* parent)
	{
		mediaKeyInterface = new OrgGnomeSettingsDaemonMediaKeysInterface(
				"org.gnome.SettingsDaemon",
				"/org/gnome/SettingsDaemon/MediaKeys",
				QDBusConnection::sessionBus(),
				parent);
	}
};

DBusMediaKeysInterfaceGnome::DBusMediaKeysInterfaceGnome(QObject* parent) :
	DBusMediaKeysInterface(parent)
{
	m = Pimpl::make<Private>(this);

	init();
}

DBusMediaKeysInterfaceGnome::~DBusMediaKeysInterfaceGnome() {}

QString DBusMediaKeysInterfaceGnome::serviceName() const
{
	return QString("org.gnome.SettingsDaemon");
}

QDBusPendingReply<> DBusMediaKeysInterfaceGnome::grabMediaKeyReply()
{
	return m->mediaKeyInterface->GrabMediaPlayerKeys("sayonara", 0);
}

void DBusMediaKeysInterfaceGnome::connectMediaKeys()
{
	connect( m->mediaKeyInterface,
			 &OrgGnomeSettingsDaemonMediaKeysInterface::MediaPlayerKeyPressed,
			 this,
			 &DBusMediaKeysInterfaceGnome::mediaKeyPressed
	);
}

