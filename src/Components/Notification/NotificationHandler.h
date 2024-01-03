
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

#ifndef NOTIFICATIONHANDLER_H
#define NOTIFICATIONHANDLER_H

#include "Notificator.h"

#include <QObject>

class NotificationHandler :
	public QObject
{
	Q_OBJECT

	signals:
		void sigNotificationsChanged();

	public:
		using QObject::QObject;

		virtual void registerNotificator(Notificator* notificator) = 0;
		virtual void changeCurrentNotificator(const QString& name) = 0;

		[[nodiscard]] virtual QList<Notificator*> notificators() const = 0;
		[[nodiscard]] virtual Notificator* currentNotificator() const = 0;

		virtual void notify(const MetaData& track) = 0;
		virtual void notify(const QString& title, const QString& message, const QString& imagePath = QString()) = 0;

		static NotificationHandler* create(QObject* parent);
};

#endif // NOTIFICATIONHANDLER_H
