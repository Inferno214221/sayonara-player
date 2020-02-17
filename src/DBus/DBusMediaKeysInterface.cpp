/* DBusMediaKeysInterface.cpp */

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

#include "DBusMediaKeysInterface.h"
#include "Utils/Logger/Logger.h"
#include "Components/PlayManager/PlayManager.h"

#include <QKeyEvent>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>


struct DBusMediaKeysInterface::Private
{
	QObject*		parent=nullptr;
	bool            initialized;
	bool			isRegistered;

	Private(QObject* parent) :
		parent(parent),
		initialized(false),
		isRegistered(false)
	{}
};

DBusMediaKeysInterface::DBusMediaKeysInterface(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(parent);
}

DBusMediaKeysInterface::~DBusMediaKeysInterface() {}

void DBusMediaKeysInterface::init()
{
	QDBusConnectionInterface* dbus_interface = QDBusConnection::sessionBus().interface();
	if (!dbus_interface->isServiceRegistered( serviceName() ))
	{
		return;
	}

	spLog(Log::Info, this) << serviceName() << " registered";

	QDBusPendingReply<> reply = grabMediaKeyReply();
	QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);

	connect(watcher, &QDBusPendingCallWatcher::finished,
			this, &DBusMediaKeysInterface::registerFinished);

	m->initialized = true;
}

bool DBusMediaKeysInterface::initialized() const
{
	return m->initialized;
}


void DBusMediaKeysInterface::mediaKeyPressed(const QString& program_name, const QString& key)
{
	Q_UNUSED(program_name)

	QKeyEvent* event = nullptr;
	auto* pm = PlayManager::instance();

	if(key.compare("play", Qt::CaseInsensitive) == 0){
		event = new QKeyEvent ( QEvent::KeyPress, Qt::Key_MediaPlay, Qt::NoModifier);
		pm->playPause();
	}

	else if(key.compare("pause", Qt::CaseInsensitive) == 0){
		event = new QKeyEvent ( QEvent::KeyPress, Qt::Key_MediaPause, Qt::NoModifier);
		pm->pause();
	}

	else if(key.compare("next", Qt::CaseInsensitive) == 0){
		event = new QKeyEvent ( QEvent::KeyPress, Qt::Key_MediaNext, Qt::NoModifier);
		pm->next();
	}

	else if(key.compare("previous", Qt::CaseInsensitive) == 0){
		event = new QKeyEvent ( QEvent::KeyPress, Qt::Key_MediaPrevious, Qt::NoModifier);
		pm->previous();
	}

	else if(key.contains("stop", Qt::CaseInsensitive) == 0){
		event = new QKeyEvent (QEvent::KeyPress, Qt::Key_MediaStop, Qt::NoModifier);
		pm->stop();
	}

	if(event && m->parent){
		QCoreApplication::postEvent (m->parent, event);
	}
}


void DBusMediaKeysInterface::registerFinished(QDBusPendingCallWatcher* watcher)
{
	QDBusMessage reply = watcher->reply();
	watcher->deleteLater();

	if (reply.type() == QDBusMessage::ErrorMessage)
	{
		spLog(Log::Warning, this) << "Cannot grab media keys: "
								   << reply.errorName() << " "
								   << reply.errorMessage();
	}

	connectMediaKeys();
}
