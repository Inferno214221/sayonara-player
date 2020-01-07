/* LibraryPluginLoader.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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
	class Container;

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
		void sig_new_library_requested(const QString& name, const QString& path);
		void sig_current_library_changed();
		void sig_libraries_changed();

	private:
		void init_libraries(const QList<Container*>& containers);
		void init_dll_libraries();

	public:
		/**
		 * @brief Search for plugins and add some predefined plugins
		 * @param containers Some predefined plugins
		 */
		void init(const QList<Container*>& containers, Container* fallback_library);

		/**
		 * @brief Get a list for all found plugins. The ui is not necessarily initialized
		 * @return list for all found library plugins
		 */
		QList<Container*>	get_libraries(bool also_empty) const;

		Container*			current_library() const;
		QWidget*			current_library_widget() const;

		void add_local_library(Container* container);
		void rename_local_library(const QString& old_name, const QString& new_name);
		void remove_local_library(const QString& name);
		void move_local_library(int old_local_library_index, int new_local_library_index);

	public slots:
		void set_current_library(const QString& name);
		void set_current_library(int index);
		void set_current_library(Container* container);
	};
}

#endif // LIBRARYPLUGINLOADER_H
