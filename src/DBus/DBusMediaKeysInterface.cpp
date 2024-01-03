/* DBusMediaKeysInterface.cpp */

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

#include "DBusMediaKeysInterface.h"

#include "Components/PlayManager/PlayManager.h"
#include "Utils/Logger/Logger.h"

#include <QKeyEvent>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

namespace Dbus
{
	struct MediaKeysInterface::Private
	{
		PlayManager* playManager;
		QObject* parent;

		Private(PlayManager* playManager, QObject* parent) :
			playManager(playManager),
			parent(parent) {}
	};

	MediaKeysInterface::MediaKeysInterface(PlayManager* playManager, QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>(playManager, parent)} {}

	MediaKeysInterface::~MediaKeysInterface() = default;

	void MediaKeysInterface::init()
	{
		auto* dbusInterface = QDBusConnection::sessionBus().interface();
		if(dbusInterface->isServiceRegistered(serviceName()))
		{
			spLog(Log::Info, this) << serviceName() << " registered";

			const auto reply = grabMediaKeyReply();
			auto* watcher = new QDBusPendingCallWatcher(reply, this);

			connect(watcher, &QDBusPendingCallWatcher::finished,
			        this, &MediaKeysInterface::registerFinished);
		}
	}

	void MediaKeysInterface::mediaKeyPressed(const QString& /*programName*/, const QString& key)
	{
		QKeyEvent* event = nullptr;

		if(key.compare("play", Qt::CaseInsensitive) == 0)
		{
			event = new QKeyEvent(QEvent::KeyPress, Qt::Key_MediaPlay, Qt::NoModifier);
			m->playManager->playPause();
		}

		else if(key.compare("pause", Qt::CaseInsensitive) == 0)
		{
			event = new QKeyEvent(QEvent::KeyPress, Qt::Key_MediaPause, Qt::NoModifier);
			m->playManager->pause();
		}

		else if(key.compare("next", Qt::CaseInsensitive) == 0)
		{
			event = new QKeyEvent(QEvent::KeyPress, Qt::Key_MediaNext, Qt::NoModifier);
			m->playManager->next();
		}

		else if(key.compare("previous", Qt::CaseInsensitive) == 0)
		{
			event = new QKeyEvent(QEvent::KeyPress, Qt::Key_MediaPrevious, Qt::NoModifier);
			m->playManager->previous();
		}

		else if(key.contains("stop", Qt::CaseInsensitive) == 0)
		{
			event = new QKeyEvent(QEvent::KeyPress, Qt::Key_MediaStop, Qt::NoModifier);
			m->playManager->stop();
		}

		if(event && m->parent)
		{
			QCoreApplication::postEvent(m->parent, event);
		}
	}

	void MediaKeysInterface::registerFinished(QDBusPendingCallWatcher* watcher)
	{
		auto reply = watcher->reply();
		watcher->deleteLater();

		if(reply.type() == QDBusMessage::ErrorMessage)
		{
			spLog(Log::Warning, this) << "Cannot grab media keys: "
			                          << reply.errorName() << " "
			                          << reply.errorMessage();
		}

		connectMediaKeys();
	}
}