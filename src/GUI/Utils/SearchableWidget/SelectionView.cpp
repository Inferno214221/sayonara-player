/* SayonaraSelectionView.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "GUI/Utils/Delegates/ComboBoxDelegate.h"

#include <QItemSelection>
#include <QKeyEvent>

#include <algorithm>

struct SelectionViewInterface::Private
{
	SelectionViewInterface::SelectionType selection_type;
	Private() :
		selection_type(SelectionViewInterface::SelectionType::Rows)
	{}
};

SelectionViewInterface::SelectionViewInterface()
{
	m = Pimpl::make<Private>();
}

SelectionViewInterface::~SelectionViewInterface() {}

void SelectionViewInterface::select_all()
{
	QItemSelectionModel* sel_model = this->selection_model();
	if(!sel_model) {
		return;
	}

	int n_rows = row_count();
	int n_cols = column_count();

	QModelIndex first_idx = model_index(0, 0);
	QModelIndex last_idx = model_index(n_rows - 1, n_cols - 1);

	QItemSelection sel = sel_model->selection();
	sel.select(first_idx, last_idx);
	sel_model->select(sel, QItemSelectionModel::ClearAndSelect);
}



void SelectionViewInterface::select_rows(const IndexSet& indexes, int min_col, int max_col)
{
	QItemSelectionModel* sel_model = this->selection_model();
	if(!sel_model){
		return;
	}

	if(indexes.empty()) {
		this->clear_selection();
		return;
	}

	if(indexes.size() > 0) {
		int first_index = indexes.first();
		this->set_current_index(first_index);
	}

	min_col = std::max(0, min_col);
	min_col = std::min(min_col, column_count() - 1);
	max_col = std::max(0, max_col);
	max_col = std::min(max_col, column_count() - 1);

	QItemSelection sel;
	if(indexes.size() == 1)
	{
		QModelIndex first_idx = model_index(indexes.first(), 0);
		QModelIndex last_idx = model_index(indexes.first(), column_count() - 1);

		sel.select(first_idx, last_idx);
		sel_model->select(sel, QItemSelectionModel::ClearAndSelect);

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
		auto other_it=it;
		auto other_predecessor=it;

		do
		{
			other_predecessor = other_it;
			other_it++;

			if(other_it == indexes.end()){
				break;
			}

		} while(*other_it - 1 == *other_predecessor);

		// select the range

		QModelIndex min_idx = model_index(*it, min_col);
		QModelIndex max_idx = model_index(*other_predecessor, max_col);
		sel.select(min_idx, max_idx);

		it = other_it;

		if(it == indexes.end()){
			break;
		}
	}

	sel_model->select(sel, QItemSelectionModel::ClearAndSelect);
}


void SelectionViewInterface::select_row(int row)
{
	select_rows({row});
}

void SelectionViewInterface::select_columns(const IndexSet& indexes, int min_row, int max_row)
{
	QItemSelectionModel* sel_model = this->selection_model();
	if(!sel_model){
		return;
	}

	QItemSelection sel;
	for(auto it = indexes.begin(); it != indexes.end(); it++){
		sel.select(model_index(min_row, *it),
				   model_index(max_row, *it));
	}

	sel_model->select(sel, QItemSelectionModel::ClearAndSelect);
}

void SelectionViewInterface::select_column(int col)
{
	IndexSet indexes(col);
	select_columns(col);
}

void SelectionViewInterface::select_items(const IndexSet& indexes)
{
	QItemSelectionModel* sel_model = this->selection_model();
	if(!sel_model){
		return;
	}

	QItemSelection sel;
	for(int index : indexes)
	{
		sel.select( model_index_by_index(index),
					model_index_by_index(index)
		);
	}

	sel_model->select(sel, QItemSelectionModel::ClearAndSelect);
}

void SelectionViewInterface::clear_selection()
{
	QItemSelectionModel* sel_model = this->selection_model();
	if(!sel_model){
		return;
	}

	sel_model->clearSelection();
}


IndexSet SelectionViewInterface::selected_items() const
{
	QItemSelectionModel* sel_model = this->selection_model();
	if(!sel_model){
		return IndexSet();
	}

	QModelIndexList idx_list = sel_model->selectedIndexes();
	IndexSet ret;
	for(auto idx : idx_list) {
		ret.insert(index_by_model_index(idx));
	}

	return ret;
}


IndexSet SelectionViewInterface::indexes_by_model_indexes(const QModelIndexList& indexes) const
{
	IndexSet ret;

	for(const QModelIndex& idx : indexes){
		ret.insert( index_by_model_index(idx) );
	}

	return ret;
}


QModelIndexList SelectionViewInterface::model_indexes_by_indexes(const IndexSet& idxs) const
{
	QModelIndexList lst;
	for(auto it = idxs.begin(); it != idxs.end(); it++){
		lst << model_index_by_index(*it);
	}
	return lst;
}


int SelectionViewInterface::min_selected_item() const
{
	IndexSet selected = selected_items();
	if(!selected.isEmpty()) {
		return *(std::min_element(selected.begin(), selected.end()));
	}

	return -1;
}


void SelectionViewInterface::set_selection_type(SelectionViewInterface::SelectionType type)
{
	m->selection_type = type;
}

SelectionViewInterface::SelectionType SelectionViewInterface::selection_type() const
{
	return m->selection_type;
}

void SelectionViewInterface::handle_key_press(QKeyEvent* e)
{
	e->setAccepted(false);

	if(this->row_count() == 0)
	{
		return;
	}

	Qt::KeyboardModifiers modifiers = e->modifiers();
	if(modifiers != Qt::NoModifier){
		return;
	}

	if(e->matches(QKeySequence::SelectAll))
	{
		this->select_all();
		e->accept();
		return;
	}

	switch(e->key())
	{
		case Qt::Key_Up:
			if(selected_items().empty())
			{
				e->accept();
				this->select_row(this->row_count() - 1);
			}

			return;

		case Qt::Key_Down:
			if(selected_items().empty())
			{
				e->accept();
				this->select_row(0);
			}

			return;

		case Qt::Key_End:
			this->select_row(this->row_count() - 1);
			e->accept();
			return;

		case Qt::Key_Home:
			this->select_row(0);
			e->accept();
			return;

		default:
			break;
	}
}
