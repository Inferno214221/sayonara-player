/* CoverView.h */

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


#ifndef ALBUMCOVERVIEW_H
#define ALBUMCOVERVIEW_H

#include "ItemView.h"

#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

#include "Gui/Utils/Widgets/Widget.h"
#include "Gui/Utils/GuiClass.h"

class QTableView;
class AbstractLibrary;
class LocalLibrary;

UI_FWD(GUI_CoverView)

namespace Library
{
	class CoverView;

	/**
	 * @brief The GUI_CoverView class
	 * @ingroup GuiLibrary
	 */
	class GUI_CoverView :
			public Gui::Widget
	{
		Q_OBJECT
		UI_CLASS(GUI_CoverView)

	signals:
		void sig_sortorder_changed(SortOrder so);
		void sig_delete_clicked();
		void sig_reload_clicked();

	public:
		explicit GUI_CoverView(QWidget* parent=nullptr);
		virtual ~GUI_CoverView() override;

		void init(LocalLibrary* library);
		bool is_initialized() const;

		IndexSet selected_items() const;
		void clear_selections() const;

	protected:
		void init_sorting_actions();
		void init_zoom_actions();

		void language_changed() override;

	private:
		void zoom_changed();
		void sortorder_changed();
		void show_artist_changed();

	private slots:
		void combo_sorting_changed(int idx);
		void combo_zoom_changed(int idx);
		void close_clicked();
		void show_artist_triggered(bool b);

		void show_utils_changed();
	};
}

#endif // ALBUMCOVERVIEW_H
