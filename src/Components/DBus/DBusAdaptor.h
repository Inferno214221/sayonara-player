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
	explicit DBusAdaptor(QStrRef object_path, QStrRef service_name, QStrRef dbus_service, QStrRef dbus_interface, QObject *parent=nullptr);
	virtual ~DBusAdaptor();

	void create_message(QString name, QVariant val);

	QString object_path() const;
	QString service_name() const;
	QString dbus_service() const;
	QString dbus_interface() const;
};


#endif // DBUSADAPTOR_H
