/* DBusMediaKeysInterface.h */

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

#ifndef DBUSMEDIAKEYSINTERFACE_H
#define DBUSMEDIAKEYSINTERFACE_H

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include "Utils/Pimpl.h"

class PlayManager;

class DBusMediaKeysInterface :
		public QObject
{
	Q_OBJECT
	PIMPL(DBusMediaKeysInterface)

public:
	explicit DBusMediaKeysInterface(PlayManager* playManager, QObject* parent=nullptr);
	virtual ~DBusMediaKeysInterface();

	void init();

protected:
	bool initialized() const;

	virtual QString serviceName() const=0;
	virtual QDBusPendingReply<> grabMediaKeyReply()=0;
	virtual void connectMediaKeys()=0;

protected slots:
	virtual void mediaKeyPressed(const QString& appName, const QString& key);
	virtual void registerFinished(QDBusPendingCallWatcher* watcher);
};

#endif // DBUSMEDIAKEYSINTERFACE_H
