
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

namespace
{
	constexpr const auto* appName = "Sayonara Player";
	constexpr const auto* serviceName = "org.freedesktop.Notifications";
	constexpr const auto* objectPath = "/org/freedesktop/Notifications";

	bool startNotificationService()
	{
		constexpr const auto* InstanceName = "DBusNotifications";

		auto interface = QDBusConnection::sessionBus().interface();
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

struct DBusNotifications::Private
{
	OrgFreedesktopNotificationsInterface interface;
	uint id {100};
	bool started;

	Private(QObject* parent) :
		interface {serviceName, objectPath, QDBusConnection::sessionBus(), parent},
		started {startNotificationService()} {}
};

DBusNotifications::DBusNotifications(NotificationHandler* notificationHandler, QObject* parent) :
	QObject(parent),
	Notificator("DBus", notificationHandler),
	m {Pimpl::make<Private>(this)} {}

DBusNotifications::~DBusNotifications() = default;

void DBusNotifications::notify(const QString& title, const QString& text, const QString& imagePath)
{
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

		QDBusPendingReply<uint> reply =
			m->interface.Notify(appName,
			                    m->id,
			                    Util::Filepath(imagePath).fileystemPath(),
			                    title,
			                    text,
			                    {},
			                    map,
			                    GetSetting(Set::Notification_Timeout));

		m->id = reply.value();
	}
}

void DBusNotifications::notify(const MetaData& track)
{
	if(m->started)
	{
		const auto active = GetSetting(Set::Notification_Show);
		if(active)
		{
			const auto coverLocation = Cover::Location::coverLocation(track);
			const auto coverPath = Util::Filepath(coverLocation.preferredPath()).fileystemPath();

			notify(track.title(), track.artist(), coverPath);
		}
	}
}
