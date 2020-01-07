/* DBusAdaptor.cpp */

/* Copyright (C) 2011-2020 Lucio Carreras
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



#include "DBusAdaptor.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QStringList>
#include <QUrl>
#include <QStringRef>

struct DBusAdaptor::Private
{
	QString		object_path;
	QString		service_name;
	QString		dbus_service;
	QString		dbus_interface;

	Private(QStrRef object_path, QStrRef service_name, QStrRef dbus_service, QStrRef dbus_interface) :
		object_path(object_path),
		service_name(service_name),
		dbus_service(dbus_service),
		dbus_interface(dbus_interface)
	{}
};

DBusAdaptor::DBusAdaptor(QStrRef object_path, QStrRef service_name, QStrRef dbus_service, QStrRef dbus_interface, QObject *parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(object_path, service_name, dbus_service, dbus_interface);
}

DBusAdaptor::~DBusAdaptor() {}

void DBusAdaptor::create_message(QString name, QVariant val)
{
	QDBusMessage sig;
	QVariantMap map;
	QVariantList args;

	map.insert(name, val);
	args << m->dbus_service << map << QStringList();

	// path, interface, name
	sig = QDBusMessage::createSignal(m->object_path, m->dbus_interface, "PropertiesChanged");
	sig.setArguments(args);

	QDBusConnection::sessionBus().send(sig);
}

QString DBusAdaptor::object_path() const
{
	return m->object_path;
}

QString DBusAdaptor::service_name() const
{
	return m->service_name;
}

QString DBusAdaptor::dbus_service() const
{
	return m->dbus_service;
}

QString DBusAdaptor::dbus_interface() const
{
	return m->dbus_interface;
}
