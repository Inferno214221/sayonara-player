/* View.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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

#include "Gui/Utils/Widgets/Dragable.h"
#include "Gui/Utils/SearchableWidget/SearchableView.h"

#include "Gui/InfoDialog/InfoDialogContainer.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"

#include "Utils/SetFwd.h"
#include "Utils/Pimpl.h"

class AbstractLibrary;

namespace Library
{
	class MergeData;
	class ItemModel;

	/**
	 * @brief The main task of the ItemView is to display a context menu
	 * for various selections. It also handles drag and drop events with
	 * a cover. It supports merging and imports
	 * @ingroup GuiLibrary
	 */
	class ItemView :
			public SearchableTableView,
			public InfoDialogContainer,
			protected Gui::Dragable
	{
		Q_OBJECT
		PIMPL(ItemView)

	signals:
		void sig_all_selected();
		void sig_delete_clicked();
		void sig_play_clicked();
		void sig_play_next_clicked();
		void sig_play_new_tab_clicked();
		void sig_append_clicked();
		void sig_refresh_clicked();
		void sig_reload_clicked();
		void sig_import_files(const QStringList& files);
		void sig_sel_changed(const IndexSet& indexes);
		void sig_merge(const Util::Set<Id>& ids, int target_id);

	private:
		ItemView(const ItemView& other)=delete;
		ItemView& operator =(const ItemView& other)=delete;

		void show_context_menu_actions(Library::ContextMenu::Entries entries);

		using SearchableTableView::set_model;

	public:
		explicit ItemView(QWidget* parent=nullptr);
		virtual ~ItemView() override;

		void set_item_model(ItemModel* model);

		virtual Library::ContextMenu::Entries context_menu_entries() const;

		/** Dragable **/
		QMimeData* dragable_mimedata() const override;
		QPixmap drag_pixmap() const override;

		void set_selection_type(SelectionViewInterface::SelectionType type) override;
		bool is_valid_drag_position(const QPoint &p) const override;

		void show_clear_button(bool visible);
		void use_clear_button(bool yesno);

		void resize_rows_to_contents();
		void resize_rows_to_contents(int first_row, int count);

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
		virtual void init_custom_context_menu(Library::ContextMenu* menu);

		Library::ContextMenu* context_menu() const;

		ItemModel* item_model() const;
		virtual AbstractLibrary* library() const;

		/**
		 * @brief indicates if multiple ids can be merged into one. For example if the same
		 * artist is written in three different ways, they can be merged to one. On the
		 * other hand, for tracks that does not make sense
		 * @return
		 */
		virtual bool is_mergeable() const=0;

		MetaDataList info_dialog_data() const override;

		virtual void selection_changed(const IndexSet& indexes);
		virtual void import_requested(const QStringList& files);

		virtual void run_merge_operation(const Library::MergeData& md);

		int viewport_height() const override;


	protected slots:
		virtual void show_context_menu(const QPoint&);
		virtual void merge_action_triggered();
		virtual void play_clicked();
		virtual void play_new_tab_clicked();
		virtual void play_next_clicked();
		virtual void delete_clicked();
		virtual void append_clicked();
		virtual void refresh_clicked();
		virtual void reload_clicked();
		virtual void cover_view_toggled();
		virtual void album_artists_toggled();
		virtual void filter_extensions_triggered(const QString& extension, bool b);
		virtual void fill();
	};
}

#endif /* ITEM_VIEW_H_ */
