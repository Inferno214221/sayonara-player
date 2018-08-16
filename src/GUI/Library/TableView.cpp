/* LibraryTableView.cpp */

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

#include "TableView.h"
#include "ItemModel.h"

#include "GUI/Library/Utils/ColumnHeader.h"
#include "GUI/Library/HeaderView.h"
#include "Utils/Settings/Settings.h"
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
	connect(m->header, &HeaderView::sectionClicked, this, &TableView::sort_by_column);
	connect(m->header, &HeaderView::sig_columns_changed, this, &TableView::header_actions_triggered);
}

TableView::~TableView() {}

void TableView::init(AbstractLibrary* library)
{
	init_view(library);

	const ColumnHeaderList headers = column_headers();
	m->shown_columns = visible_columns();
	m->sortorder = sortorder();

	QStringList header_names;
	for(ColumnHeaderPtr header : headers)
	{
		header_names << header->title();
	}

	ItemModel* model = item_model();
	model->set_header_data(header_names);

	m->header->set_column_headers(headers, m->shown_columns, m->sortorder);

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
	ColumnHeaderPtr h = m->header->column_header(idx_col);
	if(!h){
		return;
	}

	asc_sortorder = h->sortorder_asc();
	desc_sortorder = h->sortorder_desc();

	switch_sorters( m->sortorder, asc_sortorder, desc_sortorder );

	save_sortorder(m->sortorder);
}


void TableView::language_changed()
{
	ItemModel* model = item_model();
	QStringList header_names;
	for(int i=0; i<model->columnCount(); i++)
	{
		ColumnHeaderPtr header = m->header->column_header(i);
		if(header){
			header_names << header->title();
		}
	}

	model->set_header_data(header_names);
}


void TableView::resizeEvent(QResizeEvent* event)
{
	ItemView::resizeEvent(event);
	m->header->refresh_sizes(this);
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
