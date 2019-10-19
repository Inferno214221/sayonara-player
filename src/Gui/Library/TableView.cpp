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

#include "Utils/Set.h"
#include <algorithm>

using namespace Library;

struct TableView::Private
{
	HeaderView*			header=nullptr;
};

TableView::TableView(QWidget* parent) :
	Library::ItemView(parent)
{
	m = Pimpl::make<Private>();

	m->header = new HeaderView(Qt::Horizontal, this);
	setHorizontalHeader(m->header);
}

TableView::~TableView() = default;

void TableView::init(AbstractLibrary* library)
{
	init_view(library);

	const ColumnHeaderList headers = column_headers();

	{ // register names at model
		QStringList header_names;
		for(ColumnHeaderPtr header : headers)
		{
			header_names << header->title();
		}

		ItemModel* model = item_model();
		model->set_header_data(header_names);
	}

	// do this initialization here after the model knows about
	// the number of columns. Otherwise the resize column method
	// won't work
	m->header->init(headers, column_header_state(), sortorder());

	language_changed();

	connect(this, &ItemView::doubleClicked, this, &TableView::play_clicked);

	connect(m->header, &HeaderView::sig_columns_changed, this, &TableView::header_actions_triggered);
	connect(m->header, &QHeaderView::sectionClicked, this, &TableView::sort_by_column);
	connect(m->header, &QHeaderView::sectionResized, this, &TableView::section_resized);
	connect(m->header, &QHeaderView::sectionMoved, this, &TableView::section_moved);
}

void TableView::header_actions_triggered()
{
	const IndexSet sel_indexes = selected_items();

	std::for_each(sel_indexes.begin(), sel_indexes.end(), [this](int row){
		this->selectRow(row);
	});

	save_column_header_state(m->header->saveState());
}

void TableView::sort_by_column(int column_idx)
{
	Library::SortOrder sortorder = m->header->switch_sortorder(column_idx);

	apply_sortorder(sortorder);
}

void TableView::section_resized()
{
	if(!this->isVisible()){
		return;
	}

	save_column_header_state(m->header->saveState());
}

void TableView::section_moved(int logical_index, int old_visual_index, int new_visual_index)
{
	Q_UNUSED(logical_index)
	Q_UNUSED(old_visual_index)
	Q_UNUSED(new_visual_index)

	save_column_header_state(m->header->saveState());
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
