/* CoverView.h */

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

#ifndef COVERVIEW_H
#define COVERVIEW_H

#include "CoverViewSortorderInfo.h"
#include "Gui/Library/ItemView.h"
#include "Gui/Utils/SearchableWidget/SelectionView.h"
#include "Utils/Library/Sortorder.h"

class LocalLibrary;
class QAction;

namespace Library
{
	class MergeData;

	class CoverView :
		public ItemView
	{
		Q_OBJECT
		PIMPL(CoverView)

		public:
			explicit CoverView(QWidget* parent = nullptr);
			~CoverView() override;

			void init(LocalLibrary* library);
			[[nodiscard]] AbstractLibrary* library() const override;

			// QAbstractItemView
			[[nodiscard]] QStyleOptionViewItem viewOptions() const override;

			//SayonaraSelectionView
			[[nodiscard]] int mapModelIndexToIndex(const QModelIndex& idx) const override;
			[[nodiscard]] ModelIndexRange mapIndexToModelIndexes(int idx) const override;
			[[nodiscard]] SelectionViewInterface::SelectionType selectionType() const override;

			[[nodiscard]] int zoom() const;
			void changeZoom(int zoom = -1);
			void changeSortorder(AlbumSortorder so);

			[[nodiscard]] static QList<CoverViewSortorderInfo> sortingActions();
			[[nodiscard]] static QList<int> zoomFactors();

		public slots:
			void reload();
			void clearCache();

		protected:
			void fill() override;
			void initContextMenu() override;

			void languageChanged() override;

			// ItemView
			[[nodiscard]] bool isMergeable() const override;
			[[nodiscard]] MD::Interpretation metadataInterpretation() const override;
			[[nodiscard]] ItemModel* itemModel() const override;
			[[nodiscard]] int sizeHintForColumn(int size) const override;

			void wheelEvent(QWheelEvent* e) override;
			void resizeEvent(QResizeEvent* e) override;
			void hideEvent(QHideEvent* e) override;

		private:
			void resizeSections();

			// Library::ItemView
			void playClicked() override;
			void playNewTabClicked() override;
			void playNextClicked() override;
			void appendClicked() override;
			void selectedItemsChanged(const IndexSet& indexes) override;
			void refreshClicked() override;
			void runMergeOperation(const Library::MergeData& mergedata) override;
	};
}

#endif // COVERVIEW_H
