/* View.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
 *      Author: Michael Lugmair (Lucio Carreras)
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
			void sigDeleteClicked();
			void sigPlayClicked();
			void sigPlayNextClicked();
			void sigPlayNewTabClicked();
			void sigAppendClicked();
			void sigRefreshClicked();
			void sigReloadClicked();
			void sigImportFiles(const QStringList& files);
			void sigSelectionChanged(const IndexSet& indexes);

		private:
			ItemView(const ItemView& other) = delete;
			ItemView& operator=(const ItemView& other) = delete;

			void showContextMenuActions(Library::ContextMenu::Entries entries);

			using SearchableTableView::setSearchableModel;

		public:
			explicit ItemView(QWidget* parent = nullptr);
			virtual ~ItemView() override;

			void setItemModel(ItemModel* model);

			void showClearButton(bool visible);
			void useClearButton(bool yesno);

			virtual Library::ContextMenu::Entries contextMenuEntries() const;

			/** Dragable **/
			bool isValidDragPosition(const QPoint& p) const override;

		protected:
			// Events implemented in LibraryViewEvents.cpp
			virtual void mousePressEvent(QMouseEvent* event) override;
			virtual void contextMenuEvent(QContextMenuEvent* event) override;
			virtual void dragEnterEvent(QDragEnterEvent* event) override;
			virtual void dragMoveEvent(QDragMoveEvent* event) override;
			virtual void dropEvent(QDropEvent* event) override;
			virtual void resizeEvent(QResizeEvent* event) override;

			virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;

			Library::ContextMenu* contextMenu() const;
			virtual void initContextMenu();
			virtual void initCustomContextMenu(Library::ContextMenu* menu);

			ItemModel* itemModel() const;
			virtual AbstractLibrary* library() const;

			/**
			 * @brief indicates if multiple ids can be merged into one. For example if the same
			 * artist is written in three different ways, they can be merged to one. On the
			 * other hand, for tracks that does not make sense
			 * @return
			 */
			virtual bool isMergeable() const = 0;

			MetaDataList infoDialogData() const override;
			QWidget* getParentWidget() override;

			virtual void selectedItemsChanged(const IndexSet& indexes);
			virtual void importRequested(const QStringList& files);

			virtual void runMergeOperation(const Library::MergeData& md);

			int viewportHeight() const override;

		protected slots:
			virtual void showContextMenu(const QPoint&);
			virtual void mergeActionTriggered();
			virtual void playClicked();
			virtual void playNewTabClicked();
			virtual void playNextClicked();
			virtual void deleteClicked();
			virtual void appendClicked();
			virtual void refreshClicked();
			virtual void reloadClicked();
			virtual void albumArtistsToggled();
			virtual void filterExtensionsTriggered(const QString& extension, bool b);
			virtual void fill();
	};
}

#endif /* ITEM_VIEW_H_ */
