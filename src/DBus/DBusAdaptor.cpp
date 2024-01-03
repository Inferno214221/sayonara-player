/* Adapator.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include <utility>

namespace Dbus
{
	struct Adapator::Private
	{
		QString objectPath;
		QString serviceName;
		QString dbusService;
		QString dbusInterface;

		Private(QString objectPath, QString serviceName, QString dbusService, QString dbusInterface) :
			objectPath(std::move(objectPath)),
			serviceName(std::move(serviceName)),
			dbusService(std::move(dbusService)),
			dbusInterface(std::move(dbusInterface)) {}

	};

	Adapator::Adapator(const QString& objectPath, const QString& serviceName, const QString& dbusService,
	                   const QString& dbusInterface, QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>(objectPath, serviceName, dbusService, dbusInterface)} {}

	Adapator::~Adapator() = default;

	void Adapator::createMessage(const QString& name, const QVariant& val)
	{
		const auto map = QVariantMap {
			{name, val}
		};

		const auto args = QVariantList {
			m->dbusService,
			map,
			QStringList {}
		};

		auto sig = QDBusMessage::createSignal(m->objectPath, m->dbusInterface, "PropertiesChanged");
		sig.setArguments(args);

		QDBusConnection::sessionBus().send(sig);
	}

	QString Adapator::objectPath() const { return m->objectPath; }

	QString Adapator::serviceName() const { return m->serviceName; }
}