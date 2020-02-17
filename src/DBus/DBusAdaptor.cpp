/* DBusAdaptor.cpp */

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



#include "DBusAdaptor.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QStringList>
#include <QUrl>
#include <QStringRef>

struct DBusAdaptor::Private
{
	QString		objectPath;
	QString		serviceName;
	QString		dbusService;
	QString		dbusInterface;

	Private(QStrRef objectPath, QStrRef serviceName, QStrRef dbusService, QStrRef dbusInterface) :
		objectPath(objectPath),
		serviceName(serviceName),
		dbusService(dbusService),
		dbusInterface(dbusInterface)
	{}
};

DBusAdaptor::DBusAdaptor(QStrRef objectPath, QStrRef serviceName, QStrRef dbusService, QStrRef dbusInterface, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(objectPath, serviceName, dbusService, dbusInterface);
}

DBusAdaptor::~DBusAdaptor() = default;

void DBusAdaptor::createMessage(QString name, QVariant val)
{
	QDBusMessage sig;
	QVariantMap map;
	QVariantList args;

	map.insert(name, val);
	args << m->dbusService << map << QStringList();

	// path, interface, name
	sig = QDBusMessage::createSignal(m->objectPath, m->dbusInterface, "PropertiesChanged");
	sig.setArguments(args);

	QDBusConnection::sessionBus().send(sig);
}

QString DBusAdaptor::objectPath() const
{
	return m->objectPath;
}

QString DBusAdaptor::serviceName() const
{
	return m->serviceName;
}

QString DBusAdaptor::dbusService() const
{
	return m->dbusService;
}

QString DBusAdaptor::dbusInterface() const
{
	return m->dbusInterface;
}
