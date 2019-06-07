#include "DBusAdaptor.h"
#include "Components/PlayManager/PlayManager.h"

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

	PlayManagerPtr play_manager=nullptr;

	Private(QStrRef object_path, QStrRef service_name, QStrRef dbus_service, QStrRef dbus_interface) :
		object_path(object_path),
		service_name(service_name),
		dbus_service(dbus_service),
		dbus_interface(dbus_interface)
	{
		play_manager = PlayManager::instance();
	}
};

DBusAdaptor::DBusAdaptor(QStrRef object_path, QStrRef service_name, QStrRef dbus_service, QStrRef dbus_interface, QObject *parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(object_path, service_name, dbus_service, dbus_interface);
}

DBusAdaptor::~DBusAdaptor() = default;

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
