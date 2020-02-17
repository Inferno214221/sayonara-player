/* SayonaraSelectionView.cpp */

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

#include "SelectionView.h"
#include "Utils/Set.h"
#include "Utils/Algorithm.h"
#include "Gui/Utils/Delegates/ComboBoxDelegate.h"

#include <QAbstractItemView>
#include <QItemSelection>
#include <QKeyEvent>

#include <algorithm>

struct SelectionViewInterface::Private
{
	QAbstractItemView* view=nullptr;
	SelectionViewInterface* selectionViewInterface=nullptr;

	Private(SelectionViewInterface* selectionViewInterface, QAbstractItemView* view) :
		view(view),
		selectionViewInterface(selectionViewInterface)
	{}

	void selectRow(int row)
	{
		selectionViewInterface->selectRows({row});
	}

	QItemSelection getSelection() const
	{
		auto selModel = view->selectionModel();
		if(selModel)
		{
			return selModel->selection();
		}

		return QItemSelection();
	}

	void select(const QItemSelection& selection)
	{
		auto selModel = view->selectionModel();
		if(selModel)
		{
			if(selection.isEmpty())
			{
				selModel->clear();
			}

			else
			{
				selModel->select(selection, QItemSelectionModel::ClearAndSelect);
			}
		}
	}

	int rowCount()
	{
		return view->model()->rowCount();
	}

	int columnCount()
	{
		return view->model()->columnCount();
	}

	QModelIndex modelIndex(int row, int column)
	{
		return view->model()->index(row, column);
	}

	void setCurrentIndex(int index)
	{
//		ModelIndexRange range = mapIndexToModelIndexes(idx);
//		m->view->setCurrentIndex(range.first);

		view->setCurrentIndex(modelIndex(index, 0));
	}
};

SelectionViewInterface::SelectionViewInterface(QAbstractItemView* view)
{
	m = Pimpl::make<Private>(this, view);
}

SelectionViewInterface::~SelectionViewInterface() = default;

void SelectionViewInterface::selectAll()
{
	const QModelIndex firstIndex = m->modelIndex(0, 0);
	const QModelIndex lastIndex = m->modelIndex(m->rowCount() - 1, m->columnCount() - 1);

	QItemSelection selection;
	selection.select(firstIndex, lastIndex);
	m->select(selection);
}

SelectionViewInterface::SelectionType SelectionViewInterface::selectionType() const
{
	return SelectionViewInterface::SelectionType::Rows;
}

void SelectionViewInterface::selectRows(const IndexSet& indexes, int minColumn, int maxColumn)
{
	if(indexes.empty())
	{
		m->select(QItemSelection());
		return;
	}

	if(minColumn == -1 || minColumn >= m->columnCount()){
		minColumn = 0;
	}

	if(maxColumn == -1 || maxColumn >= m->columnCount()){
		maxColumn = m->columnCount() - 1;
	}

	QItemSelection selection;
	if(indexes.size() == 1)
	{
		ModelIndexRange range = mapIndexToModelIndexes(indexes.first());
		selection.select(range.first, range.second);
		m->select(selection);

		return;
	}

	// the goal is: find consecutive ranges.
	// For every select or merge an overlap is
	// tested, that will last really long when
	// there are already around 500 items.
	// probably, the runtime is O(n^2)

	// the list is pre-sorted (see the fill-function)

	// we start at the very beginning (i)
	// let j run until the first element that is not
	// consecutive.

	// our range is from i to the last known j.
	// count down the j by one. So the worst that can
	// happen, is that j is as big as i again.
	// i is increased in the next loop, so progress is
	// guaranteed


	for(auto it=indexes.begin(); it!=indexes.end(); it++)
	{
		auto otherIt=it;
		auto otherPredecessor=it;

		do
		{
			otherPredecessor = otherIt;
			otherIt++;

			if(otherIt == indexes.end()){
				break;
			}

		} while(*otherIt - 1 == *otherPredecessor);

		// select the range

		QModelIndex minIndex = m->modelIndex(*it, minColumn);
		QModelIndex maxIndex = m->modelIndex(*otherPredecessor, maxColumn);
		selection.select(minIndex, maxIndex);

		it = otherIt;

		if(it == indexes.end()){
			break;
		}
	}

	m->select(selection);
}


void SelectionViewInterface::selectColumns(const IndexSet& indexes, int min_row, int max_row)
{
	QItemSelection sel;
	for(auto it = indexes.begin(); it != indexes.end(); it++)
	{
		sel.select(m->modelIndex(min_row, *it),
				   m->modelIndex(max_row, *it));
	}

	m->select(sel);
}

void SelectionViewInterface::selectItems(const IndexSet& indexes)
{
	QItemSelection selection;
	for(int index : indexes)
	{
		ModelIndexRange range = mapIndexToModelIndexes(index);
		selection.select(range.first, range.second);
	}

	m->select(selection);
}

IndexSet SelectionViewInterface::selectedItems() const
{
	const QModelIndexList indexList = m->getSelection().indexes();

	IndexSet ret;
	for(auto idx : indexList)
	{
		int row = mapModelIndexToIndex(idx);
		if(!ret.contains(row))
		{
			ret.insert(row);
		}
	}

	return ret;
}

IndexSet SelectionViewInterface::mapModelIndexesToIndexes(const QModelIndexList& indexes) const
{
	IndexSet ret;

	for(const QModelIndex& idx : indexes){
		ret.insert( mapModelIndexToIndex(idx) );
	}

	return ret;
}

ModelIndexRanges SelectionViewInterface::mapIndexesToModelIndexRanges(const IndexSet& idxs) const
{
	ModelIndexRanges lst;
	for(auto it = idxs.begin(); it != idxs.end(); it++){
		lst << mapIndexToModelIndexes(*it);
	}
	return lst;
}

void SelectionViewInterface::handleKeyPress(QKeyEvent* e)
{
	e->setAccepted(false);

	if(m->rowCount() == 0)
	{
		return;
	}

	Qt::KeyboardModifiers modifiers = e->modifiers();
	auto blupp = modifiers & (Qt::ControlModifier|Qt::AltModifier|Qt::MetaModifier);
	if(blupp != 0)
	{
		return;
	}

	if(e->matches(QKeySequence::SelectAll))
	{
		this->selectAll();
		e->accept();
		return;
	}

	switch(e->key())
	{
		case Qt::Key_Up:
			if(selectedItems().empty())
			{
				e->accept();
				m->selectRow(m->rowCount() - 1);
			}

			return;

		case Qt::Key_Down:
			if(selectedItems().empty())
			{
				e->accept();
				m->selectRow(0);
			}

			return;

		case Qt::Key_End:
			m->selectRow(m->rowCount() - 1);
			e->accept();
			return;

		case Qt::Key_Home:
			m->selectRow(0);
			e->accept();
			return;

		default:
			break;
	}
}
