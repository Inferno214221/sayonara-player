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

#include "Gui/Library/ItemModel.h"
#include "Gui/Library/Header/ColumnHeader.h"
#include "Gui/Library/Header/HeaderView.h"

#include "Utils/Set.h"
#include "Utils/Algorithm.h"

using namespace Library;

struct TableView::Private
{
	HeaderView*	header=nullptr;
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

	ColumnHeaderList headers = columnHeaders();
	Util::Algorithm::sort(headers, [](ColumnHeaderPtr p1, ColumnHeaderPtr p2){
		return (p1->columnIndex() < p2->columnIndex());
	});

	m->header->init(headers, columnHeaderState(), sortorder());

	connect(m->header, &QHeaderView::sectionCountChanged, this, &TableView::headerColumnsChanged);
	connect(m->header, &QHeaderView::sectionResized, this, &TableView::sectionResized);
	connect(m->header, &QHeaderView::sectionMoved, this, &TableView::sectionMoved);
	connect(m->header, &QHeaderView::sortIndicatorChanged, this, &TableView::sortorderChanged);

	languageChanged();
}

void TableView::headerColumnsChanged(int /*oldCount*/, int /*newCount*/)
{
	const IndexSet selectedIndexes = selectedItems();
	for(int index : selectedIndexes)
	{
		this->selectRow(index);
	}

	setupColumnNames();
	saveColumnHeaderState(m->header->saveState());
}

void TableView::sortorderChanged(int index, Qt::SortOrder qtSortorder)
{
	applySortorder(m->header->sortorder(index, qtSortorder));
}

void TableView::sectionResized(int /*logicalIndex*/, int /*oldSize*/, int /*newSize*/)
{
	if(!this->isVisible()){
		return;
	}

	saveColumnHeaderState(m->header->saveState());
}

void TableView::sectionMoved(int /*logicalIndex*/, int /*oldVisualIndex*/, int /*newVisualIndex*/)
{
	setupColumnNames();
	saveColumnHeaderState(m->header->saveState());
}

void TableView::setupColumnNames()
{
	int count = columnHeaders().count();
	for(int i=0; i<count; i++)
	{
		QString text = m->header->columnText(i);
		model()->setHeaderData(i, Qt::Horizontal, text, Qt::DisplayRole);
	}
}

void TableView::languageChanged()
{
	setupColumnNames();
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
