/* GenreViewContextMenu.h */

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

#ifndef GENREVIEWCONTEXTMENU_H
#define GENREVIEWCONTEXTMENU_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/ContextMenu/ContextMenu.h"

namespace Library
{
	/**
	 * @brief Context Menu for the tree view.
	 * Notification of the tree view action is done
	 * by using a bool setting listener to Set::Lib_GenreTree
	 * So there's not signal for it. Everything else can be
	 * accessed using the methods of Gui::ContextMenu
	 * @ingroup GuiLibrary
	 */
	class GenreViewContextMenu :
		public Gui::ContextMenu
	{
		Q_OBJECT
		PIMPL(GenreViewContextMenu)

		public:
			GenreViewContextMenu(QWidget* parent=nullptr);
			~GenreViewContextMenu() override;

		private slots:
			void toggleTreeTriggered();

		protected:
			void languageChanged() override;
	};
}

#endif // GENREVIEWCONTEXTMENU_H
