/* SayonaraSelectionView.cpp */

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

#include "SelectionView.h"
#include "Utils/Algorithm.h"
#include "Utils/Set.h"

#include <QAbstractItemView>
#include <QItemSelection>
#include <QKeyEvent>

namespace
{
	QItemSelection getSelection(QAbstractItemView* view)
	{
		const auto* selectionModel = view->selectionModel();
		return selectionModel
		       ? selectionModel->selection()
		       : QItemSelection {};
	}

	void select(QAbstractItemView* view, const QItemSelection& selection)
	{
		auto* selectionModel = view->selectionModel();
		if(selectionModel)
		{
			if(selection.isEmpty())
			{
				selectionModel->clear();
			}

			else
			{
				selectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
			}
		}
	}

	int rowCount(QAbstractItemView* view) { return view->model()->rowCount(); }

	int columnCount(QAbstractItemView* view) { return view->model()->columnCount(); }

	QModelIndex modelIndex(QAbstractItemView* view, const int row, const int column)
	{
		return view->model()->index(row, column);
	}

	using Range = QPair<int, int>;

	QList<Range> partitionIndexSet(const IndexSet& indexSet)
	{
		if(indexSet.isEmpty())
		{
			return {};
		}

		auto result = QList<QPair<int, int>> {
			QPair<int, int> {indexSet.first(), indexSet.first()}
		};

		auto& currentRange = result.last();
		for(const auto& index: indexSet)
		{
			auto& lastElement = currentRange.second;
			if(lastElement == (index - 1))
			{
				currentRange.second = index;
			}

			else if(lastElement < (index - 1))
			{
				result << QPair {index, index};
				currentRange = result.last();
			}
		}

		return result;
	}

	QPair<int, int> checkMinMaxColumnBoundaries(QAbstractItemView* view, const int minColumn, const int maxColumn)
	{
		auto result = QPair<int, int> {minColumn, maxColumn};
		if((minColumn == -1) || (minColumn >= columnCount(view)))
		{
			result.first = 0;
		}

		if((maxColumn == -1) || (maxColumn >= columnCount(view)))
		{
			result.second = columnCount(view) - 1;
		}

		return result;
	}
}

struct SelectionViewInterface::Private
{
	QAbstractItemView* view;

	explicit Private(QAbstractItemView* view) :
		view(view) {}
};

SelectionViewInterface::SelectionViewInterface(QAbstractItemView* view) :
	m {Pimpl::make<Private>(view)} {}

SelectionViewInterface::~SelectionViewInterface() = default;

void SelectionViewInterface::selectAll()
{
	const auto firstIndex = modelIndex(m->view, 0, 0);
	const auto lastIndex = modelIndex(m->view, rowCount(m->view) - 1, columnCount(m->view) - 1);

	auto selection = QItemSelection {};
	selection.select(firstIndex, lastIndex);
	select(m->view, selection);
}

SelectionViewInterface::SelectionType SelectionViewInterface::selectionType() const
{
	return SelectionViewInterface::SelectionType::Rows;
}

void SelectionViewInterface::selectRows(const IndexSet& indexes, const int minColumn, const int maxColumn)
{
	const auto minMaxColumns = checkMinMaxColumnBoundaries(m->view, minColumn, maxColumn);
	const auto ranges = partitionIndexSet(indexes);

	auto selection = QItemSelection {};
	for(const auto& range: ranges)
	{
		selection.select(
			modelIndex(m->view, range.first, minMaxColumns.first),
			modelIndex(m->view, range.second, minMaxColumns.second));
	}

	select(m->view, selection);
}

void SelectionViewInterface::selectItems(const IndexSet& indexes)
{
	auto selection = QItemSelection {};
	for(const auto index: indexes)
	{
		const auto range = mapIndexToModelIndexes(index);
		selection.select(range.first, range.second);
	}

	select(m->view, selection);
}

IndexSet SelectionViewInterface::selectedItems() const
{
	if(!m->view || !m->view->model())
	{
		return {};
	}

	const auto indexList = getSelection(m->view).indexes();

	auto result = IndexSet {};
	for(const auto& modelIndex: indexList)
	{
		if(modelIndex.isValid())
		{
			if(const auto index = mapModelIndexToIndex(modelIndex); index >= 0)
			{
				result.insert(index);
			}
		}
	}

	return result;
}

bool SelectionViewInterface::handleKeyPress(QKeyEvent* e)
{
	const auto modifierMask = e->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);

	if((rowCount(m->view) == 0) || (modifierMask != 0))
	{
		return false;
	}

	if(e->matches(QKeySequence::SelectAll))
	{
		selectAll();
		return true;
	}

	switch(e->key())
	{
		case Qt::Key_Up:
			if(selectedItems().empty())
			{
				selectRows({rowCount(m->view) - 1});
				return true;
			}

			return false;

		case Qt::Key_Down:
			if(selectedItems().empty())
			{
				selectRows({0});
				return true;
			}

			return false;

		case Qt::Key_End:
			selectRows({rowCount(m->view) - 1});
			return true;

		case Qt::Key_Home:
			selectRows({0});
			return true;

		case Qt::Key_Escape:
			if(!selectedItems().empty())
			{
				m->view->clearSelection();
				return true;
			}

			return false;

		default:
			return false;
	}
}
