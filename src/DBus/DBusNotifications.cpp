
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
#include "Utils/Macros.h"
#include "Utils/Filepath.h"

#include <QDir>

struct DBusNotifications::Private
{
	OrgFreedesktopNotificationsInterface* interface=nullptr;
	MetaData					md;
	uint						id;
	Cover::Location				cl;
	QMap<QString, QString>		resource_file_map; // map resource file image paths to real paths

	Private() : id(100) {}
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

DBusNotifications::~DBusNotifications() = default;

void DBusNotifications::notify(const QString& title, const QString& text, const QString& image_path)
{
	QVariantMap map;
	map.insert("action-icons", false);
	map.insert("desktop-entry",
		QDir(SAYONARA_INSTALL_SHARE_PATH).absoluteFilePath("applications/sayonara.desktop")
	);
	map.insert("resident", false);
	map.insert("sound-file", QString());
	map.insert("sound-name", QString());
	map.insert("suppress-sound", true);
	map.insert("transient", false);
	map.insert("urgency", 1);

	QDBusPendingReply<uint> reply =
	m->interface->Notify("Sayonara Player",
	   m->id,
	   Util::Filepath(image_path).filesystem_path(),
	   title,
	   text,
	   QStringList(),
	   map,
	   GetSetting(Set::Notification_Timeout)
	);

	m->id = reply.value();
}

QString DBusNotifications::name() const
{
	return "DBus";
}

void DBusNotifications::notify(const MetaData& md)
{
	m->md = md;

	bool active = GetSetting(Set::Notification_Show);
	if(!active){
		return;
	}

	m->cl = Cover::Location::cover_location(md);
	QString path = Util::Filepath(m->cl.preferred_path()).filesystem_path();

	notify(m->md.title(), m->md.artist(), path);
}
