/* Proxy.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#ifndef PROXY_H
#define PROXY_H

#include "Utils/Singleton.h"

#include <QObject>

class QString;
class Proxy :
		public QObject
{
	Q_OBJECT
	SINGLETON(Proxy)

	private slots:
		void proxy_changed();

	public:
		void init();

		QString hostname() const;
		int port() const;
		QString username() const;
		QString password() const;
		bool active() const;
		bool has_username() const;

		QString full_url() const;
};

#endif // PROXY_H
