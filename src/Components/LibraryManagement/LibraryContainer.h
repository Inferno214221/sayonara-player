/* LibraryContainer.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef SAYONARA_PLAYER_COMPONENTS_LIBRARY_CONTAINER_H
#define SAYONARA_PLAYER_COMPONENTS_LIBRARY_CONTAINER_H

class QWidget;
class QFrame;
class QIcon;
class QMenu;
class QAction;

#include <QObject>

namespace Library
{
	class LibraryContainer
	{
		public:
			virtual ~LibraryContainer() = default;

			[[nodiscard]] virtual QFrame* header() const = 0;
			[[nodiscard]] virtual QIcon icon() const = 0;
			[[nodiscard]] virtual QMenu* menu() = 0;
			[[nodiscard]] virtual QString displayName() const = 0;
			[[nodiscard]] virtual QString name() const = 0;
			[[nodiscard]] virtual QWidget* widget() const = 0;
			[[nodiscard]] virtual bool isLocal() const = 0;
			virtual void init() = 0;
			virtual void rename(const QString& newName) = 0;
	};
}

using LibraryContainerInterface = Library::LibraryContainer;

Q_DECLARE_INTERFACE(LibraryContainerInterface, "com.sayonara-player.library")

#endif // SAYONARA_PLAYER_COMPONENTS_LIBRARY_CONTAINER_H
