/* AlbumView.h */

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

#ifndef LIBRARYVIEWALBUM_H
#define LIBRARYVIEWALBUM_H

#include "Utils/MetaData/Album.h"
#include "GUI/Library/TableView.h"
#include <QList>
#include <QModelIndex>

class DiscPopupMenu;

namespace Library
{
	class AlbumView :
			public Library::TableView
	{
		Q_OBJECT
		PIMPL(AlbumView)

	signals:
		void sig_disc_pressed(int);

	protected slots:
		void index_clicked(const QModelIndex& idx);
		void albums_ready();

	public:
		explicit AlbumView(QWidget *parent=nullptr);
		virtual ~AlbumView();

	private:
		// Library::TableView
		void init_view(AbstractLibrary* library) override;
		ColumnHeaderList column_headers() const override;

		BoolList visible_columns() const override;
		void save_visible_columns(const BoolList& lst) override;

		SortOrder sortorder() const override;
		void save_sortorder(SortOrder s) override;

		// Library::ItemView
		void play_clicked() override;
		void play_new_tab_clicked() override;
		void play_next_clicked() override;
		void append_clicked() override;
		void selection_changed(const IndexSet& indexes) override;
		void refresh_clicked() override;
		void run_merge_operation(const MergeData& mergedata) override;

		void clear_discnumbers();
		void add_discnumbers(const QList<Disc>& dns);
		void calc_discmenu_point(QModelIndex idx);
		void delete_discmenu();
		void init_discmenu(QModelIndex idx);
		void show_discmenu();
		void show_context_menu(const QPoint& p) override;

		AbstractLibrary* library() const override;

	private slots:
		void use_clear_button_changed();

	public:
		template <typename T, typename ModelType>
		void fill(const T& input_data)
		{
			clear_discnumbers();

			for(const Album& album : input_data){
				add_discnumbers(album.discnumbers);
			}

			ItemView::fill<T, ModelType>(input_data);
		}
	};
}

#endif // LIBRARYVIEWALBUM_H
