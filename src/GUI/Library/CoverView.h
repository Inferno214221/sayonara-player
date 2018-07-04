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

#include "GUI/Library/ItemView.h"
#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

class AbstractLibrary;

namespace Library
{
	class CoverModel;
	class CoverView :
			public Library::ItemView
	{
		Q_OBJECT
		PIMPL(CoverView)

	public:
		explicit CoverView(AbstractLibrary* library, QWidget* topbar, QWidget* parent=nullptr);
		virtual ~CoverView();

		void refresh();
		void language_changed() override;


	protected:
		void wheelEvent(QWheelEvent* e) override;
		void resizeEvent(QResizeEvent* e) override;

		QStyleOptionViewItem viewOptions() const override;

		void init_context_menu() override;
		void init_sorting_actions();
		void init_zoom_actions();

		//SayonaraSelectionView
		int index_by_model_index(const QModelIndex& idx) const override;
		QModelIndex model_index_by_index(int idx) const override;

	private:
		void change_zoom(int zoom=-1);
		void middle_clicked() override;
		void play_next_clicked() override;
		void append_clicked() override;
		void selection_changed(const IndexSet& indexes) override;


	private slots:
		void albums_ready();
		void timed_out();
		void change_sortorder(::Library::SortOrder so);
		void menu_sorting_triggered();
		void combo_sorting_changed(int idx);
		void combo_zoom_changed(int idx);
		void show_utils_triggered();
		void cover_changed();

		void double_clicked(const QModelIndex& index);
	};
}

#endif // ALBUMCOVERVIEW_H
