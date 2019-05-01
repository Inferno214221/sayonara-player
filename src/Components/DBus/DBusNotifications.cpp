
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

#include "DBusNotifications.h"
#include "Components/Covers/CoverLocation.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

struct DBusNotifications::Private
{
	OrgFreedesktopNotificationsInterface* interface=nullptr;
	MetaData md;
	Cover::Location cl;
};

DBusNotifications::DBusNotifications(QObject* parent) :
	QObject(parent),
	NotificationInterface()
{
	m = Pimpl::make<Private>();

	QString service_name = "org.freedesktop.Notifications";
	m->interface = new OrgFreedesktopNotificationsInterface(
				QString(service_name),
				QString("/org/freedesktop/Notifications"),
				QDBusConnection::sessionBus(),
				parent
	);


	QDBusConnection bus = QDBusConnection::sessionBus();
	QDBusConnectionInterface* dbus_interface = bus.interface();
	dbus_interface->startService(service_name);

	if (!dbus_interface->isServiceRegistered(service_name))
	{
		sp_log(Log::Warning, this) << service_name << " not registered";
	}

	else{
		sp_log(Log::Info, this) << " registered";
	}

	NotificationHandler::instance()->register_notificator(this);
}

DBusNotifications::~DBusNotifications() {}

void DBusNotifications::notify(const QString& title, const QString& text, const QString& image_path)
{
	QVariantMap map;
	map.insert("action-icons", false);
	map.insert("desktop-entry", "/usr/share/applications/sayonara.desktop");
	map.insert("resident", false);
	map.insert("sound-file", QString());
	map.insert("sound-name", QString());
	map.insert("suppress-sound", true);
	map.insert("transient", false);
	map.insert("urgency", 1);

	m->interface->Notify("Sayonara Player",
	   500,
	   image_path,
	   title,
	   text,
	   QStringList(),
	   map,
	   GetSetting(Set::Notification_Timeout)
	);
}

QString DBusNotifications::name() const
{
	return "DBus";
}

void DBusNotifications::notify(const MetaData& md)
{
	this->track_changed(md);
}


void DBusNotifications::track_changed(const MetaData& md)
{
	m->md = md;

	bool active = GetSetting(Set::Notification_Show);
	if(!active){
		return;
	}

	m->cl = Cover::Location::cover_location(md);
	QString path = m->cl.preferred_path();

	notify(m->md.title(), " by " + m->md.artist(), path);
}
