/* GUILibraryInfoBox.h

 * Copyright (C) 2011-2020 Lucio Carreras
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Lucio Carreras,
 * Sep 2, 2012
 *
 */

#ifndef GUILIBRARYINFOBOX_H_
#define GUILIBRARYINFOBOX_H_

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_LibraryInfoBox)

namespace Library
{
	/**
	 * @brief A info box containing library statistics
	 * @ingroup Gui::Library
	 */
	class GUI_LibraryInfoBox :
			public Gui::Dialog
	{
		Q_OBJECT
		UI_CLASS(GUI_LibraryInfoBox)
		PIMPL(GUI_LibraryInfoBox)

	public:
		explicit GUI_LibraryInfoBox(LibraryId library_id, QWidget* parent=nullptr);
		~GUI_LibraryInfoBox() override;

	protected:
		void language_changed() override;
		void skin_changed() override;
		void showEvent(QShowEvent* e) override;
		void refresh();
	};
}

#endif /* GUILIBRARYINFOBOX_H_ */
