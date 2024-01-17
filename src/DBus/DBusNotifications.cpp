
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

#include "DBusNotifications.h"
#include "Components/Covers/CoverLocation.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Macros.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Filepath.h"

#include <QDir>
#include <QTimer>

namespace
{
	constexpr const auto* appName = "Sayonara Player";
	constexpr const auto* serviceName = "org.freedesktop.Notifications";
	constexpr const auto* objectPath = "/org/freedesktop/Notifications";

	bool startNotificationService()
	{
		constexpr const auto* InstanceName = "DBusNotifications";

		auto* interface = QDBusConnection::sessionBus().interface();
		interface->startService(serviceName);

		const auto success = interface->isServiceRegistered(serviceName);
		if(!success)
		{
			spLog(Log::Warning, InstanceName) << serviceName << " not registered";
		}

		else
		{
			spLog(Log::Debug, InstanceName) << " registered";
		}

		return success;
	}
}

namespace Dbus
{
	struct Notifications::Private
	{
		QTimer* timer;
		OrgFreedesktopNotificationsInterface interface;
		uint id {0}; // NOLINT(readability-magic-numbers)
		bool started;

		explicit Private(QObject* parent) :
			timer {new QTimer(parent)},
			interface {serviceName, objectPath, QDBusConnection::sessionBus(), parent},
			started {startNotificationService()} {}
	};

	Notifications::Notifications(NotificationHandler* notificationHandler, QObject* parent) :
		QObject(parent),
		Notificator("DBus", notificationHandler),
		m {Pimpl::make<Private>(this)}
	{
		connect(m->timer, &QTimer::timeout, this, &Notifications::closeCurrentNotification);
	}

	Notifications::~Notifications()
	{
		closeCurrentNotification();
	}

	void Notifications::notify(const QString& title, const QString& text, const QString& imagePath)
	{
		closeCurrentNotification();

		if(m->started)
		{
			const auto desktopFile = Util::Filepath {":/Desktop/com.sayonara-player.Sayonara.desktop"};

			const auto map = QVariantMap {
				{"action-icons",   false},
				{"desktop-entry",  desktopFile.fileystemPath()},
				{"resident",       false},
				{"sound-file",     QString {}},
				{"sound-name",     QString {}},
				{"suppress-sound", true},
				{"transient",      false},
				{"urgency",        1}
			};

			const auto cleanImagePath = Util::Filepath(imagePath).fileystemPath();
			const auto timeout = GetSetting(Set::Notification_Timeout);
			QDBusPendingReply<uint> reply =
				m->interface.Notify(appName, m->id, cleanImagePath, title, text, {}, map, timeout);

			m->id = reply.value();
			m->timer->start(timeout);
		}
	}

	void Notifications::notify(const MetaData& track)
	{
		if(m->started)
		{
			if(GetSetting(Set::Notification_Show))
			{
				const auto coverLocation = Cover::Location::coverLocation(track);
				const auto coverPath = Util::Filepath(coverLocation.preferredPath()).fileystemPath();

				notify(track.title(), track.artist(), coverPath);
			}
		}
	}

	void Notifications::closeCurrentNotification()
	{
		if(m->id > 0)
		{
			m->interface.CloseNotification(m->id);
			m->id = 0;
		}
	}
}