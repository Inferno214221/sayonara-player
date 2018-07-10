/* CoverView.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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


#ifndef ALBUMCOVERVIEW_H
#define ALBUMCOVERVIEW_H

#include "ItemView.h"

#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

#include "GUI/Utils/Widgets/Widget.h"
#include "GUI/Utils/GuiClass.h"

class AbstractLibrary;
class LocalLibrary;

UI_FWD(GUI_CoverView)

namespace Library
{
	class CoverModel;
	class GUI_CoverView :
			public Gui::Widget
	{
		Q_OBJECT
		UI_CLASS(GUI_CoverView)

	public:
		explicit GUI_CoverView(QWidget* parent=nullptr);
		virtual ~GUI_CoverView();

		Library::ItemView* cover_view();
		void init(LocalLibrary* library);

		void refresh();

	protected:
		void init_sorting_actions();
		void init_zoom_actions();
		void language_changed() override;

	private:
		void zoom_changed(int zoom);
		void sortorder_changed(Library::SortOrder so);

	private slots:
		void combo_sorting_changed(int idx);
		void combo_zoom_changed(int idx);

		void show_utils_changed();
	};
}

#endif // ALBUMCOVERVIEW_H
