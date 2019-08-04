/* DBusAdaptor.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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



#ifndef DBUSADAPTOR_H
#define DBUSADAPTOR_H

#include <QObject>
#include "Utils/Pimpl.h"

using QStrRef=const QString&;

class DBusAdaptor :
		public QObject
{
	Q_OBJECT
	PIMPL(DBusAdaptor)

protected:
	DBusAdaptor(QStrRef object_path, QStrRef service_name, QStrRef dbus_service, QStrRef dbus_interface, QObject *parent=nullptr);
	virtual ~DBusAdaptor();

	void create_message(QString name, QVariant val);

	QString object_path() const;
	QString service_name() const;
	QString dbus_service() const;
	QString dbus_interface() const;
};


#endif // DBUSADAPTOR_H
