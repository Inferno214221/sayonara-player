/* LibraryTableView.cpp */

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
	initView(library);

	const ColumnHeaderList headers = columnHeaders();
	for(int i=0; i<headers.size(); i++)
	{
		itemModel()->setHeaderData(i, Qt::Horizontal, headers[i]->title(), Qt::DisplayRole);
	}

	// do this initialization here after the model knows about
	// the number of columns. Otherwise the resize column method
	// won't work
	m->header->init(headers, columnHeaderState(), sortorder());

	languageChanged();

	connect(this, &ItemView::doubleClicked, this, &TableView::playClicked);

	connect(m->header, &HeaderView::sigColumnsChanged, this, &TableView::headerActionsTriggered);
	connect(m->header, &QHeaderView::sectionClicked, this, &TableView::sortByColumn);
	connect(m->header, &QHeaderView::sectionResized, this, &TableView::sectionResized);
	connect(m->header, &QHeaderView::sectionMoved, this, &TableView::sectionMoved);
}

void TableView::headerActionsTriggered()
{
	const IndexSet selectedIndexes = selectedItems();
	for(int index : selectedIndexes)
	{
		this->selectRow(index);
	}

	saveColumnHeaderState(m->header->saveState());
}

void TableView::sortByColumn(int column_idx)
{
	Library::SortOrder sortorder = m->header->switchSortorder(column_idx);

	applySortorder(sortorder);
}

void TableView::sectionResized()
{
	if(!this->isVisible()){
		return;
	}

	saveColumnHeaderState(m->header->saveState());
}

void TableView::sectionMoved(int logical_index, int old_visual_index, int new_visual_index)
{
	Q_UNUSED(logical_index)
	Q_UNUSED(old_visual_index)
	Q_UNUSED(new_visual_index)

	saveColumnHeaderState(m->header->saveState());
}

void TableView::languageChanged()
{
	ItemModel* model = itemModel();

	for(int i=0; i<model->columnCount(); i++)
	{
		ColumnHeaderPtr header = m->header->column(i);
		if(header) {
			model->setHeaderData(i, Qt::Horizontal, header->title(), Qt::DisplayRole);
		}
	}
}

int TableView::mapModelIndexToIndex(const QModelIndex& idx) const
{
	return idx.row();
}

ModelIndexRange TableView::mapIndexToModelIndexes(int idx) const
{
	return ModelIndexRange(itemModel()->index(idx, 0),
						   itemModel()->index(idx, itemModel()->columnCount() - 1));
}
