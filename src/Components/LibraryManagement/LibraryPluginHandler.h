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

#include <QList>
#include <QObject>

#include <memory>

class QMenu;
class QString;
class QWidget;

namespace Library
{
	class Info;
	class LibraryContainer;

	class PluginHandler :
		public QObject
	{
		Q_OBJECT

		signals:
			void sigNewLibraryRequested(const QString& name, const QString& path);
			void sigCurrentLibraryChanged();
			void sigLibrariesChanged();

		public:
			~PluginHandler() override;

			virtual void init(const QList<LibraryContainer*>& containers, LibraryContainer* fallbackLibrary) = 0;

			[[nodiscard]] virtual QList<LibraryContainer*> libraries(bool alsoEmpty) const = 0;
			[[nodiscard]] virtual LibraryContainer* currentLibrary() const = 0;
			[[nodiscard]] virtual QWidget* currentLibraryWidget() const = 0;

			// LocalLibraryWatcher
			virtual void addLocalLibrary(LibraryContainer* container) = 0;
			virtual void renameLocalLibrary(const QString& oldName, const QString& newName) = 0;
			virtual void removeLocalLibrary(const QString& name) = 0;
			virtual void moveLocalLibrary(int oldIndex, int newIndex) = 0;

			static PluginHandler* create();

		public slots: // NOLINT(readability-redundant-access-specifiers)
			virtual void setCurrentLibrary(const QString& name) = 0;
			virtual void setCurrentLibrary(int index) = 0;
			virtual void setCurrentLibrary(LibraryContainer* currentLibrary) = 0;
	};

}

#endif // LIBRARYPLUGINLOADER_H
