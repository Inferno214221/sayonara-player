/* View.h */

/* Copyright (C) 2011-2017 Lucio Carreras
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


/*
 * MyListView.h
 *
 *  Created on: Jun 26, 2011
 *      Author: Lucio Carreras
 */

#ifndef ITEM_VIEW_H_
#define ITEM_VIEW_H_

#include "GUI/Utils/Widgets/Dragable.h"
#include "GUI/Utils/SearchableWidget/SearchableView.h"

#include "GUI/InfoDialog/InfoDialogContainer.h"
#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"
#include "GUI/Utils/Shortcuts/ShortcutWidget.h"

#include "Utils/Library/Sortorder.h"
#include "Utils/MetaData/MetaDataFwd.h"
#include "Utils/typedefs.h"
#include "Utils/Set.h"
#include "Utils/Pimpl.h"

class AbstractLibrary;
class LibraryContextMenu;
class QStringList;
class QMenu;

namespace Library
{
	class ColumnHeaderList;

	class ItemModel;

	class ItemView :
			public SearchableTableView,
			public InfoDialogContainer,
			protected Dragable,
			protected ShortcutWidget
	{
		Q_OBJECT
		PIMPL(ItemView)

	protected:
		struct MergeData
		{
			SP::Set<Id>	source_ids;
			Id			target_id;

			bool is_valid() const;
		};

	signals:
		void sig_all_selected();
		void sig_delete_clicked();
		void sig_play_clicked();
		void sig_play_next_clicked();
		void sig_play_new_tab_clicked();
		void sig_append_clicked();
		void sig_refresh_clicked();
		void sig_import_files(const QStringList& files);
		void sig_sel_changed(const IndexSet& indexes);
		void sig_merge(const SP::Set<Id>& ids, int target_id);

	private:
		ItemView(const ItemView& other)=delete;
		ItemView& operator =(const ItemView& other)=delete;

		void show_context_menu_actions(LibraryContextMenu::Entries entries);

		using SearchableTableView::set_model;

	public:
		explicit ItemView(QWidget* parent=nullptr);
		virtual ~ItemView();

		void set_item_model(ItemModel* model);

		virtual LibraryContextMenu::Entries context_menu_entries() const;


		/** Dragable **/
		QMimeData* dragable_mimedata() const override;
		QPixmap drag_pixmap() const override;

		void set_metadata_interpretation(MD::Interpretation type);
		void set_selection_type(SelectionViewInterface::SelectionType type) override;

		void show_clear_button(bool visible);
		void use_clear_button(bool yesno);

		bool is_valid_drag_position(const QPoint &p) const override;


	protected:
		// Events implemented in LibraryViewEvents.cpp
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void contextMenuEvent(QContextMenuEvent* event) override;

		virtual void dragEnterEvent(QDragEnterEvent *event) override;
		virtual void dragMoveEvent(QDragMoveEvent *event) override;
		virtual void dropEvent(QDropEvent* event) override;
		virtual void changeEvent(QEvent* event) override;
		virtual void keyPressEvent(QKeyEvent* event) override;
		virtual void resizeEvent(QResizeEvent *event) override;

		virtual void selected_items_changed (const QItemSelection& selected, const QItemSelection& deselected );

		virtual void init_context_menu();
		virtual void init_context_menu_custom_type(LibraryContextMenu* menu);

		LibraryContextMenu* context_menu() const;

		ItemModel* item_model() const;
		virtual AbstractLibrary* library() const;

		// InfoDialogContainer
		virtual MD::Interpretation metadata_interpretation() const override final;
		MetaDataList info_dialog_data() const override;

		virtual void selection_changed(const IndexSet& indexes);
		virtual void import_requested(const QStringList& files);

		MergeData calc_mergedata() const;
		virtual void run_merge_operation(const MergeData& md);


	protected slots:
		virtual void show_context_menu(const QPoint&);
		virtual void merge_action_triggered();
		virtual void play_clicked();
		virtual void play_new_tab_clicked();
		virtual void play_next_clicked();
		virtual void delete_clicked();
		virtual void append_clicked();
		virtual void refresh_clicked();
		virtual void fill();

	public:
		void resize_rows_to_contents();
		void resize_rows_to_contents(int first_row, int count);

		QString get_shortcut_text(const QString& shortcut_identifier) const override;
	};
}

#endif /* ITEM_VIEW_H_ */
