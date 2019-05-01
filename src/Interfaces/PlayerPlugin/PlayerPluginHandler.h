/* PlayerPluginHandler.h */

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

#ifndef PLAYERPLUGINHANDLER_H
#define PLAYERPLUGINHANDLER_H

#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QObject>

namespace PlayerPlugin
{
	class Base;

	class Handler :
		public QObject
	{
		Q_OBJECT
		PIMPL(Handler)
		SINGLETON(Handler)

	signals:
		void sig_plugin_added(PlayerPlugin::Base* plugin);
		void sig_plugin_closed();
		void sig_plugin_action_triggered(bool b);

	private:
		void plugin_opened(Base* p);

	private slots:
		void plugin_action_triggered(bool b);
		void plugin_opened();
		void plugin_closed();
		void language_changed();

	public:
		void add_plugin(Base* plugin);
		void show_plugin(const QString& name);

		Base*			find_plugin(const QString& name);
		QList<Base*>	all_plugins() const;
		Base*			current_plugin() const;
	};
}

#endif // PLAYERPLUGINHANDLER_H
