/* LibraryFileExtensionBar.h */

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

#ifndef LIBRARYFILEEXTENSIONBAR_H
#define LIBRARYFILEEXTENSIONBAR_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

namespace Gui
{
	class ExtensionSet;
}

class AbstractLibrary;

namespace Library
{
	/**
	 * @brief The FileExtensionBar class
	 * @ingroup GuiLibrary
	 */
	class FileExtensionBar :
		public Gui::Widget
	{
		Q_OBJECT
		PIMPL(FileExtensionBar)

		signals:
			void sigCloseClicked();

		public:
			explicit FileExtensionBar(QWidget* parent = nullptr);
			~FileExtensionBar() override;

			/**
			 * @brief This method needs to be called before any other operation
			 * @param library
			 */
			void init(AbstractLibrary* library);

			/**
			 * @brief refetches all tracks with extensions, clears the layout,
			 * adds new buttons
			 */
			void refresh();

			/**
			 * @brief clears all buttons and the layout
			 */
			void clear();

			/**
			 * @brief has_extensions
			 * @return true, if there's more than one extension. false else
			 */
			bool hasExtensions() const;

		protected:
			void languageChanged() override;

		private slots:
			void buttonToggled(bool b);
			void closeClicked();
	};
}

#endif // LIBRARYFILEEXTENSIONBAR_H
