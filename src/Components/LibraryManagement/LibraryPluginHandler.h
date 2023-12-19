/* LibraryPluginLoader.h */

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

#ifndef LIBRARYPLUGINLOADER_H
#define LIBRARYPLUGINLOADER_H

#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QObject>

class QMenu;
namespace Library
{
	class Info;
	class LibraryContainer;

	/**
	 * @brief Library Plugin Manager
	 * @ingroup LibraryPlugins
	 */
	class PluginHandler :
		public QObject
	{
		Q_OBJECT
		PIMPL(PluginHandler)
			SINGLETON(PluginHandler)

		signals:
			void sigNewLibraryRequested(const QString& name, const QString& path);
			void sigCurrentLibraryChanged();
			void sigLibrariesChanged();

		private:
			void initLibraries(const QList<LibraryContainer*>& containers);
			void initDllLibraries();

		public:
			/**
			 * @brief Search for plugins and add some predefined plugins
			 * @param containers Some predefined plugins
			 */
			void init(const QList<LibraryContainer*>& containers, LibraryContainer* fallback_library);
			void shutdown();

			/**
			 * @brief Get a list for all found plugins. The ui is not necessarily initialized
			 * @return list for all found library plugins
			 */
			QList<LibraryContainer*> libraries(bool also_empty) const;

			LibraryContainer* currentLibrary() const;
			QWidget* currentLibraryWidget() const;

			void addLocalLibrary(LibraryContainer* container);
			void renameLocalLibrary(const QString& old_name, const QString& new_name);
			void removeLocalLibrary(const QString& name);
			void moveLocalLibrary(int old_local_library_index, int new_local_library_index);

		public slots:
			void setCurrentLibrary(const QString& name);
			void setCurrentLibrary(int index);
			void setCurrentLibrary(LibraryContainer* container);
	};
}

#endif // LIBRARYPLUGINLOADER_H
