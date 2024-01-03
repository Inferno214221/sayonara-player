
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

#ifndef DBUSNOTIFICATIONS_H
#define DBUSNOTIFICATIONS_H

#include "DBus/Notifications.h"
#include "Components/Notification/NotificationHandler.h"
#include "Utils/Pimpl.h"

class NotificationHandler;
namespace Dbus
{
	class Notifications :
		public QObject,
		public Notificator
	{
		Q_OBJECT
		PIMPL(Notifications)

		public:
			explicit Notifications(NotificationHandler* notificationHandler, QObject* parent = nullptr);
			~Notifications() override;

			void notify(const MetaData& track) override;
			void notify(const QString& title, const QString& text, const QString& imagePath) override;
	};
}

#endif // DBUSNOTIFICATIONS_H
