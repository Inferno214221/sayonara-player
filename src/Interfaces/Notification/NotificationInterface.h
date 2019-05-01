
/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef ABSTRACTNOTIFICATOR_H
#define ABSTRACTNOTIFICATOR_H

#include <QString>
#include "Utils/Pimpl.h"
/**
 * @brief The NotificationInterface class
 * @ingroup Interfaces
 */
class MetaData;
class NotificationInterface
{
public:
	/**
	 * @brief Constructor
	 * @param name appearing in GUI_Notifications
	 */
	explicit NotificationInterface();

	virtual ~NotificationInterface();

	/**
	 * @brief notify when Metadata has been changed
	 * @param md
	 */
	virtual void notify(const MetaData& md)=0;

	/**
	 * @brief show standard notification
	 * @param title Notification title
	 * @param message Notification message
	 * @param image_path Notification image
	 */
	virtual void notify(const QString& title, const QString& message, const QString& image_path=QString())=0;

	/**
	 * @brief get name of notification interface
	 * @return
	 */
	virtual QString name() const=0;
	virtual QString display_name() const;
};

using NotificatonList=QList<NotificationInterface*>;

#endif // ABSTRACTNOTIFICATOR_H
