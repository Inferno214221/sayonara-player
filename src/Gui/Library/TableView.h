/* TableView.h */

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

#ifndef LIBRARYTABLEVIEW_H
#define LIBRARYTABLEVIEW_H

#include "ItemView.h"
#include "Gui/Library/Header/ColumnHeader.h"

#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

namespace Library
{
	/**
	 * @brief The TableView class
	 * @ingroup GuiLibrary
	 */
	class TableView :
		public ItemView
	{
		Q_OBJECT
		PIMPL(TableView)

	signals:
		void sig_sortorder_changed(SortOrder);

	private:
		TableView(const TableView& other)=delete;
		TableView& operator=(const TableView& other)=delete;

	public:
		explicit TableView(QWidget* parent=nullptr);
		virtual ~TableView() override;

		virtual void init(AbstractLibrary* library);

	protected:
		/**
		 * @brief here, the model and delegate should be instantiated as well as
		 * connections and setting listeners
		 * @param library
		 */
		virtual void init_view(AbstractLibrary* library)=0;

		/**
		 * @brief returns a list of ColumnHeader objects containing name,
		 * sortorder. Everytime when the language is changed, this method is
		 * fetched as well when the UI is instantiated
		 * @return
		 */
		virtual ColumnHeaderList column_headers() const=0;

		/**
		 * @brief This method returns the SAVED column header sizes as
		 * they were remembered since the last time sayonara was running.
		 * Also see save_column_header_sizes()
		 * @return a list of widths in pixels
		 */
		virtual QByteArray column_header_state() const=0;

		/**
		 * @brief Here, the column headers sizes should be saved somewhere
		 * @param a list of widths in pixels. The list should be as big as
		 * there are columns (even if they are not visible)
		 */
		virtual void save_column_header_state(const QByteArray& state)=0;

		/**
		 * @brief returns the current sortorder for the table view
		 */
		virtual SortOrder sortorder() const=0;

		/**
		 * @brief saves the current sortorder
		 */
		virtual void apply_sortorder(SortOrder s)=0;

		void language_changed() override;

		// SayonaraSelectionView.h
		int index_by_model_index(const QModelIndex& idx) const override;
		ModelIndexRange model_indexrange_by_index(int idx) const override;

	protected slots:
		void header_actions_triggered();
		void sort_by_column(int column_idx);
		void section_resized();
		void section_moved(int logical_index, int old_visual_index, int new_visual_index);
	};
}

#endif // LIBRARYTABLEVIEW_H
