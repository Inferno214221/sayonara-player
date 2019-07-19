/* LibraryTableView.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "TableView.h"
#include "ItemModel.h"

#include "Gui/Library/Header/ColumnHeader.h"
#include "Gui/Library/Header/HeaderView.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Set.h"

#include <algorithm>

using namespace Library;

template <typename T>
void switch_sorters(T& srcdst, T src1, T src2)
{
	if(srcdst == src1) {
		srcdst = src2;
	}

	else {
		srcdst = src1;
	}
}

struct TableView::Private
{
	BoolList			shown_columns;
	HeaderView*			header=nullptr;
	Library::SortOrder  sortorder;
};

TableView::TableView(QWidget* parent) :
	Library::ItemView(parent)
{
	m = Pimpl::make<Private>();

	m->header = new HeaderView(Qt::Horizontal, this);
	setHorizontalHeader(m->header);

	connect(this, &ItemView::doubleClicked, this, &TableView::play_clicked);

	connect(m->header, &HeaderView::sig_columns_changed, this, &TableView::header_actions_triggered);
	connect(m->header, &QHeaderView::sectionClicked, this, &TableView::sort_by_column);
	connect(m->header, &QHeaderView::sectionResized, this, &TableView::sizes_changed);
}

TableView::~TableView() = default;

void TableView::init(AbstractLibrary* library)
{
	init_view(library);

	ColumnHeaderList headers = column_headers();
	IntList sizes = column_header_sizes();

	if(headers.size() == sizes.size())
	{
		for(int i=0; i<sizes.size(); i++)
		{
			headers.at(i)->set_preferred_size(sizes.at(i));
		}
	}

	m->shown_columns = visible_columns();
	m->sortorder = sortorder();

	QStringList header_names;
	for(ColumnHeaderPtr header : headers)
	{
		header->preferred_size();
		header_names << header->title();
	}

	ItemModel* model = item_model();
	model->set_header_data(header_names);

	m->header->set_columns(headers, m->shown_columns, m->sortorder);

	language_changed();
}



void TableView::header_actions_triggered()
{
	IndexSet sel_indexes = selected_items();

	std::for_each(sel_indexes.begin(), sel_indexes.end(), [this](int row){
		this->selectRow(row);
	});

	m->shown_columns = m->header->shown_columns();

	save_visible_columns(m->shown_columns);
}


void TableView::sort_by_column(int column_idx)
{
	Library::SortOrder asc_sortorder, desc_sortorder;

	int idx_col = m->header->visualIndex(column_idx);
	ColumnHeaderPtr h = m->header->column(idx_col);
	if(!h){
		return;
	}

	asc_sortorder = h->sortorder_asc();
	desc_sortorder = h->sortorder_desc();

	switch_sorters( m->sortorder, asc_sortorder, desc_sortorder );

	save_sortorder(m->sortorder);
}

void TableView::sizes_changed()
{
	if(!this->isVisible()){
		return;
	}

	IntList sizes;
	for(int i=0; i<this->column_count(); i++)
	{
		sizes << this->horizontalHeader()->sectionSize(i);
	}

	save_column_header_sizes(sizes);
}

void TableView::language_changed()
{
	ItemModel* model = item_model();
	QStringList header_names;
	for(int i=0; i<model->columnCount(); i++)
	{
		ColumnHeaderPtr header = m->header->column(i);
		if(header){
			header_names << header->title();
		}
	}

	model->set_header_data(header_names);
}


int TableView::index_by_model_index(const QModelIndex& idx) const
{
	return idx.row();
}

ModelIndexRange TableView::model_indexrange_by_index(int idx) const
{
	return ModelIndexRange(item_model()->index(idx, 0),
						   item_model()->index(idx, item_model()->columnCount() - 1));
}
