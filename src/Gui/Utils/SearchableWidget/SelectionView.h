/* SayonaraSelectionView.h */

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

#ifndef SAYONARASELECTIONVIEW_H
#define SAYONARASELECTIONVIEW_H

#include "Utils/Pimpl.h"
#include <QModelIndex>

using ModelIndexRange = QPair<QModelIndex, QModelIndex>; // top left, bottom right

class QAbstractItemView;
class QItemSelection;
class QItemSelectionModel;
class QKeyEvent;

class SelectionViewInterface
{
	PIMPL(SelectionViewInterface)

	public:
		enum class SelectionType
		{
			Rows = 0,
			Items
		};

		virtual ~SelectionViewInterface();

		[[nodiscard]] virtual IndexSet selectedItems() const;

	protected:
		explicit SelectionViewInterface(QAbstractItemView* view);

		void selectRows(const IndexSet& rows, int minimumColumn = -1, int maximumColumn = -1);
		void selectItems(const IndexSet& indexes);
		void selectAll();

		[[nodiscard]] virtual SelectionViewInterface::SelectionType selectionType() const;
		[[nodiscard]] virtual int mapModelIndexToIndex(const QModelIndex& idx) const = 0;
		[[nodiscard]] virtual ModelIndexRange mapIndexToModelIndexes(int idx) const = 0;

		virtual bool handleKeyPress(QKeyEvent* e);
};

#endif // SAYONARASELECTIONVIEW_H
