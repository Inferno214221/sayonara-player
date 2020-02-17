
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

#ifndef NOTIFICATIONHANDLER_H
#define NOTIFICATIONHANDLER_H

#include "NotificationInterface.h"
#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QObject>

class DummyNotificator : public NotificationInterface
{
	public:
		explicit DummyNotificator();
		virtual ~DummyNotificator() override;

		virtual void notify(const MetaData& md) override;
		virtual void notify(const QString& title, const QString& message, const QString& imagePath) override;

		QString name() const override;
};

class NotificationHandler :
		public QObject
{
	Q_OBJECT
	SINGLETON_QOBJECT(NotificationHandler)
	PIMPL(NotificationHandler)

	signals:
		// emitted when some AbstractNotifcator registered itself
		void sigNotificationsChanged();

	private:
		NotificationInterface* get() const;

	public:
		void registerNotificator(NotificationInterface* notificator);
		void notificatorChanged(const QString& name);

		int currentIndex() const;

		NotificatonList notificators() const;

		virtual void notify(const MetaData& md);
		virtual void notify(const QString& title, const QString& message, const QString& imagePath=QString());
};

#endif // NOTIFICATIONHANDLER_H
