/* ArtistView.h */

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

#ifndef ARTISTVIEW_H
#define ARTISTVIEW_H

#include "TableView.h"
#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

namespace Library
{
	class MergeData;

	/**
	 * @brief The ArtistView class
	 * @ingroup GuiLibrary
	 */
	class ArtistView :
			public TableView
	{
		PIMPL(ArtistView)
		public:
			explicit ArtistView(QWidget* parent=nullptr);
			~ArtistView() override;

		// ItemView interface
		protected:
			AbstractLibrary* library() const override;
			void selection_changed(const IndexSet& indexes) override;
			void play_next_clicked() override;
			void append_clicked() override;
			void refresh_clicked() override;
			void play_clicked() override;
			void play_new_tab_clicked() override;
			void run_merge_operation(const Library::MergeData& mergedata) override;

			void init_view(AbstractLibrary* library) override;
			void init_context_menu() override;

			ColumnHeaderList column_headers() const override;
			QByteArray column_header_state() const override;
			void save_column_header_state(const QByteArray& state) override;

			SortOrder sortorder() const override;
			void apply_sortorder(SortOrder s) override;

			// ItemView
			bool is_mergeable() const override;
			MD::Interpretation metadata_interpretation() const override;

			void language_changed() override;

		private slots:
			void use_clear_button_changed();
			void album_artists_changed();
			void album_artists_triggered(bool b);
	};
}

#endif // ARTISTVIEW_H
