/* AlbumView.h */

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

#ifndef LIBRARYVIEWALBUM_H
#define LIBRARYVIEWALBUM_H

#include "TableView.h"
#include "Utils/MetaData/Album.h"

#include <QList>

namespace Library
{
	class DiscPopupMenu;
	class MergeData;

	class AlbumView :
		public TableView
	{
		Q_OBJECT
		PIMPL(AlbumView)

		signals:
			void sigDiscPressed(Disc d);

		public:
			explicit AlbumView(QWidget* parent = nullptr);
			~AlbumView() override;

		protected:
			void initView(AbstractLibrary* library) override;
			[[nodiscard]] ItemModel* itemModel() const override;

			[[nodiscard]] ColumnHeaderList columnHeaders() const override;
			[[nodiscard]] QByteArray columnHeaderState() const override;
			void saveColumnHeaderState(const QByteArray& state) override;
			[[nodiscard]] VariableSortorder sortorder() const override;
			void applySortorder(VariableSortorder s) override;

			[[nodiscard]] bool autoResizeState() const override;
			void saveAutoResizeState(bool b) override;

			[[nodiscard]] AbstractLibrary* library() const override;
			[[nodiscard]] PlayActionEventHandler::TrackSet trackSet() const override;
			void triggerSelectionChange(const IndexSet& indexes) override;
			void refreshView() override;

			[[nodiscard]] bool isMergeable() const override;
			void runMergeOperation(const MergeData& mergedata) override;

			[[nodiscard]] MD::Interpretation metadataInterpretation() const override;

			void showContextMenu(const QPoint& p) override;

		private slots:
			void indexClicked(const QModelIndex& idx);
			void useClearButtonChanged();

		private: // NOLINT(*-redundant-access-specifiers)
			void calcDiscmenuPoint(const QModelIndex& idx);
			void deleteDiscmenu();
			void initDiscmenu(const QModelIndex& idx);
			void showDiscmenu();
	};
}

#endif // LIBRARYVIEWALBUM_H
