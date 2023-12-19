/* SomaFMLibraryContainer.h */

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


/* SomaFMLibraryContainer.h */

#ifndef GUI_SOMAFMLIBRARY_CONTAINER_H
#define GUI_SOMAFMLIBRARY_CONTAINER_H

#include <QtGlobal>
#include "Gui/Library/LibraryContainer.h"

namespace Library
{
	class PluginHandler;
}

namespace SomaFM
{
	class Library;
	class GUI_SomaFM;

	class LibraryContainer :
		public Gui::Library::Container
	{
		Q_OBJECT
		PIMPL(LibraryContainer)

		private:
			GUI_SomaFM* ui = nullptr;

		public:
			LibraryContainer(SomaFM::Library* library, ::Library::PluginHandler* pluginHandler);
			~LibraryContainer() override;

			// override from LibraryViewInterface
			[[nodiscard]] QString name() const override;
			[[nodiscard]] QString displayName() const override;
			[[nodiscard]] QWidget* widget() const override;
			[[nodiscard]] QIcon icon() const override;
			[[nodiscard]] QMenu* menu() override;
			[[nodiscard]] QFrame* header() const override;
			void initUi() override;
	};
}
#endif
