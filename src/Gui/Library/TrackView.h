/* TrackView.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include "TableView.h"
#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

class AbstractLibrary;
namespace Library
{
	class TrackView :
		public TableView
	{
		Q_OBJECT
		PIMPL(TrackView)

	public:
		explicit TrackView(QWidget* parent=nullptr);
		~TrackView();

	private:
		AbstractLibrary* library() const override;
		//from Library::TableView
		void init_view(AbstractLibrary* library) override;
		ColumnHeaderList column_headers() const override;
		IntList column_header_sizes() const override;
		void save_column_header_sizes(const IntList& sizes) override;

		SortOrder sortorder() const override;
		void save_sortorder(SortOrder s) override;

		BoolList visible_columns() const override;
		void save_visible_columns(const BoolList& lst) override;

		LibraryContextMenu::Entries context_menu_entries() const override;

		// from Library::ItemView
		void play_clicked() override;
		void play_new_tab_clicked() override;
		void play_next_clicked() override;
		void append_clicked() override;
		void selection_changed(const IndexSet& lst) override;
		void refresh_clicked() override;

		bool is_mergeable() const override;
		MD::Interpretation metadata_interpretation() const override;
	};
}

#endif // TRACKVIEW_H
