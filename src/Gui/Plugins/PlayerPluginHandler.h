/* PlayerPluginHandler.h */

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
			void sigPluginAdded(PlayerPlugin::Base* plugin);
			void sigPluginActionTriggered(bool b);

		public:
			void addPlugin(Base* plugin);
			void showPlugin(const QString& name);

			Base* findPlugin(const QString& name);
			Base* currentPlugin() const;
			QList<Base*> allPlugins() const;

			void shutdown();

		private slots:
			void pluginActionTriggered(bool b);
			void languageChanged();
	};
}

#endif // PLAYERPLUGINHANDLER_H
