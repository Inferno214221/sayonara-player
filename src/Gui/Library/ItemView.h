/* View.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "Components/Library/PlayActionEventHandler.h"

#include "Gui/Utils/Widgets/Dragable.h"
#include "Gui/Utils/SearchableWidget/SearchableView.h"
#include "Gui/Utils/SearchableWidget/SelectionView.h"

#include "Gui/InfoDialog/InfoDialogContainer.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"

#include "Utils/SetFwd.h"
#include "Utils/Pimpl.h"

class AbstractLibrary;

namespace Library
{
	class MergeData;
	class ItemModel;
	class PlayActionEventHandler;

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

		public:
			explicit ItemView(QWidget* parent = nullptr);
			~ItemView() override;

			ItemView(const ItemView& other) = delete;
			ItemView& operator=(const ItemView& other) = delete;

			void showClearButton(bool visible);
			void useClearButton(bool yesno);

			[[nodiscard]] virtual Library::ContextMenu::Entries contextMenuEntries() const;

			[[nodiscard]] bool isValidDragPosition(const QPoint& p) const override;

		protected:
			void init(const std::shared_ptr<PlayActionEventHandler>& playActionEventHandler);
			void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;

			[[nodiscard]] Library::ContextMenu* contextMenu() const;
			virtual void initContextMenu();
			virtual void initCustomContextMenu(Library::ContextMenu* menu);

			[[nodiscard]] SearchModel* searchModel() const override;
			[[nodiscard]] virtual ItemModel* itemModel() const = 0;
			[[nodiscard]] virtual AbstractLibrary* library() const;

			[[nodiscard]] virtual bool isMergeable() const = 0;

			[[nodiscard]] MetaDataList infoDialogData() const override;
			QWidget* getParentWidget() override;

			virtual void selectedItemsChanged(const IndexSet& indexes);
			virtual void importRequested(const QStringList& files);

			virtual void runMergeOperation(const Library::MergeData& md);

			[[nodiscard]] QRect viewportGeometry() const override;

			void mousePressEvent(QMouseEvent* event) override;
			void contextMenuEvent(QContextMenuEvent* event) override;
			void dragEnterEvent(QDragEnterEvent* event) override;
			void dragMoveEvent(QDragMoveEvent* event) override;
			void dropEvent(QDropEvent* event) override;
			void resizeEvent(QResizeEvent* event) override;

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

		private:
			void showContextMenuActions(Library::ContextMenu::Entries entries);
	};
}

#endif /* ITEM_VIEW_H_ */
