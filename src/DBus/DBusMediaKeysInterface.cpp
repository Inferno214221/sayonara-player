/* DBusMediaKeysInterface.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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
	bool			is_registered;

	Private(QObject *parent) :
		parent(parent),
		initialized(false),
		is_registered(false)
	{}
};

DBusMediaKeysInterface::DBusMediaKeysInterface(QObject *parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(parent);
}

DBusMediaKeysInterface::~DBusMediaKeysInterface() {}

void DBusMediaKeysInterface::init()
{
	QDBusConnectionInterface* dbus_interface = QDBusConnection::sessionBus().interface();
	if (!dbus_interface->isServiceRegistered( service_name() ))
	{
		return;
	}

	sp_log(Log::Info, this) << service_name() << " registered";

	QDBusPendingReply<> reply = grab_media_key_reply();
	QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);

	connect(watcher, &QDBusPendingCallWatcher::finished,
			this, &DBusMediaKeysInterface::sl_register_finished);

	m->initialized = true;
}

bool DBusMediaKeysInterface::initialized() const
{
	return m->initialized;
}


void DBusMediaKeysInterface::sl_media_key_pressed(const QString& program_name, const QString& key)
{
	Q_UNUSED(program_name)

	QKeyEvent* event = nullptr;
	auto* pm = PlayManager::instance();

	if(key.compare("play", Qt::CaseInsensitive) == 0){
		event = new QKeyEvent ( QEvent::KeyPress, Qt::Key_MediaPlay, Qt::NoModifier);
		pm->play_pause();
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


void DBusMediaKeysInterface::sl_register_finished(QDBusPendingCallWatcher* watcher)
{
	QDBusMessage reply = watcher->reply();
	watcher->deleteLater();

	if (reply.type() == QDBusMessage::ErrorMessage)
	{
		sp_log(Log::Warning, this) << "Cannot grab media keys: "
								   << reply.errorName() << " "
								   << reply.errorMessage();
	}

	connect_media_keys();
}
