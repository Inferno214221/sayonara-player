/* LocalLibraryContainer.h */

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

#ifndef LOCALLIBRARYCONTAINER_H
#define LOCALLIBRARYCONTAINER_H

#include "Gui/Library/LibraryContainer.h"
#include "Utils/Pimpl.h"

namespace Library
{
	class Info;
	class Manager;
	class PluginHandler;
}

class LocalLibraryContainer :
	public Gui::Library::Container
{
	PIMPL(LocalLibraryContainer)

	public:
		LocalLibraryContainer(Library::Manager* libraryManager, const Library::Info& library,
		                      Library::PluginHandler* pluginHandler);
		~LocalLibraryContainer() override;

		[[nodiscard]] QFrame* header() const override;
		[[nodiscard]] QIcon icon() const override;
		[[nodiscard]] QMenu* menu() override;
		[[nodiscard]] QString displayName() const override;
		[[nodiscard]] QString name() const override;
		[[nodiscard]] QWidget* widget() const override;
		[[nodiscard]] bool isLocal() const override;
		void initUi() override;
		void rename(const QString& new_name) override;
};

#endif // LOCALLIBRARYCONTAINER_H
