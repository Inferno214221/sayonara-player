/* SayonaraSelectionView.h */

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

#ifndef SAYONARASELECTIONVIEW_H
#define SAYONARASELECTIONVIEW_H

#include "Utils/Pimpl.h"
#include <QModelIndex>

using ModelIndexRange=QPair<QModelIndex, QModelIndex>; // top left, bottom right
using ModelIndexRanges=QList<ModelIndexRange>;

class QAbstractItemView;
class QItemSelection;
class QItemSelectionModel;
class QKeyEvent;

/**
 * @brief The SayonaraSelectionView class
 * @ingroup Searchable
 */
class SelectionViewInterface
{
	PIMPL(SelectionViewInterface)

public:
	enum class SelectionType
	{
		Rows=0,
		Columns,
		Items
	};

	virtual IndexSet selectedItems() const;

protected:
	SelectionViewInterface(QAbstractItemView* view);
	virtual ~SelectionViewInterface();

	void selectRows(const IndexSet& rows, int minimumColumn=-1, int maximumColumn=-1);
	void selectColumns(const IndexSet& columns, int minimumRow=-1, int maximumRow=-1);
	void selectItems(const IndexSet& indexes);
	void selectAll();

	virtual SelectionViewInterface::SelectionType selectionType() const;

	virtual int mapModelIndexToIndex(const QModelIndex& idx) const=0;
	virtual ModelIndexRange mapIndexToModelIndexes(int idx) const=0;

	IndexSet mapModelIndexesToIndexes(const QModelIndexList& indexes) const;
	ModelIndexRanges mapIndexesToModelIndexRanges(const IndexSet& indexes) const;

protected:
	virtual bool handleKeyPress(QKeyEvent* e);
};

#endif // SAYONARASELECTIONVIEW_H
