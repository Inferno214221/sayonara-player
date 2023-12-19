/* LibraryContainerImpl.h */

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

#ifndef SAYONARA_PLAYER_GUI_LIBRARY_CONTAINER_H
#define SAYONARA_PLAYER_GUI_LIBRARY_CONTAINER_H

#include "Utils/Pimpl.h"
#include "Components/LibraryManagement/LibraryContainer.h"

namespace Library
{
	class PluginHandler;
}

namespace Gui::Library
{
	class Container :
		public QObject,
		public ::Library::LibraryContainer
	{
		Q_OBJECT
		PIMPL(Container)

			friend class PluginHandler;

		public:
			explicit Container(::Library::PluginHandler* libraryPluginHandler);
			~Container() override;

			void init() override;

			void rename(const QString& new_name) override;
			[[nodiscard]] QString displayName() const override;
			[[nodiscard]] QMenu* menu() override;
			[[nodiscard]] bool isLocal() const override;

		protected:
			virtual void initUi() = 0;
	};
}

#endif // SAYONARA_PLAYER_GUI_LIBRARY_CONTAINER_H
