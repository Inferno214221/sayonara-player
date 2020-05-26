/* AlbumView.h */

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

#ifndef LIBRARYVIEWALBUM_H
#define LIBRARYVIEWALBUM_H

#include "TableView.h"
#include "Utils/MetaData/Album.h"

#include <QList>

namespace Library
{
	class DiscPopupMenu;

	/**
	 * @brief The AlbumView class
	 * @ingroup GuiLibrary
	 */
	class AlbumView :
		public TableView
	{
		Q_OBJECT
		PIMPL(AlbumView)

		signals:
			void sigDiscPressed(Disc d);

		protected slots:
			void indexClicked(const QModelIndex& idx);

		public:
			explicit AlbumView(QWidget* parent = nullptr);
			virtual ~AlbumView() override;

		protected:
			ColumnHeaderList columnHeaders() const override;
			QByteArray columnHeaderState() const override;
			void saveColumnHeaderState(const QByteArray& state) override;

		private:
			// Library::TableView
			void initView(AbstractLibrary* library) override;

			SortOrder sortorder() const override;
			void applySortorder(SortOrder s) override;

			// Library::ItemView
			void playClicked() override;
			void playNewTabClicked() override;
			void playNextClicked() override;
			void appendClicked() override;
			void selectedItemsChanged(const IndexSet& indexes) override;
			void refreshClicked() override;
			void runMergeOperation(const MergeData& mergedata) override;
			bool isMergeable() const override;
			MD::Interpretation metadataInterpretation() const override;

			bool autoResizeState() const override;
			void saveAutoResizeState(bool b) override;

			void calcDiscmenuPoint(QModelIndex idx);
			void deleteDiscmenu();
			void initDiscmenu(QModelIndex idx);

			void showDiscmenu();
			void showContextMenu(const QPoint& p) override;

			AbstractLibrary* library() const override;

		private slots:
			void useClearButtonChanged();
	};
}

#endif // LIBRARYVIEWALBUM_H
