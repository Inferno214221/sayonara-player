/* ArtistView.h */

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



#ifndef ARTISTVIEW_H
#define ARTISTVIEW_H

#include "TableView.h"
#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

namespace Library
{
	class ArtistView :
			public Library::TableView
	{
		PIMPL(ArtistView)
		public:
			explicit ArtistView(QWidget* parent=nullptr);
			~ArtistView();

		// ItemView interface
		protected:
			void selection_changed(const IndexSet& indexes) override;
			void merge_action_triggered() override;
			void play_next_clicked() override;
			void middle_clicked() override;
			void append_clicked() override;
			void refresh_clicked() override;
			void play_clicked() override;
			void play_new_tab_clicked() override;

		// TableView interface
		protected:
			void init_view(AbstractLibrary* library) override;
			ColumnHeaderList column_headers() const override;

			BoolList visible_columns() const override;
			void save_visible_columns(const BoolList& columns) override;

			Library::SortOrder sortorder() const override;
			void save_sortorder(Library::SortOrder s) override;

		private slots:
			void double_clicked(const QModelIndex& index);
			void artists_ready();
			void use_clear_button_changed();
	};
}

#endif // ARTISTVIEW_H
