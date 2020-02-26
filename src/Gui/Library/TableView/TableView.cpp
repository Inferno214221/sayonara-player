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
	QByteArray headerState = columnHeaderState();
	initView(library);

	ColumnHeaderList headers = columnHeaders();

	// make sure that the headers are sorted due to their enum value
	// otherwise header names are mapped in a wrong way in the model
	Util::Algorithm::sort(headers, [](ColumnHeaderPtr a1, ColumnHeaderPtr a2)
	{
		return (a1->type() < a2->type());
	});

	for(int i=0; i<headers.size(); i++)
	{
		itemModel()->setHeaderData(i, Qt::Horizontal, headers[i]->title(), Qt::DisplayRole);
	}

	languageChanged();

	connect(this, &ItemView::doubleClicked, this, &TableView::playClicked);

	connect(m->header, &HeaderView::sigColumnsChanged, this, &TableView::headerActionsTriggered);
	connect(m->header, &QHeaderView::sectionClicked, this, &TableView::sortByColumn);
	connect(m->header, &QHeaderView::sectionResized, this, &TableView::sectionResized);
	connect(m->header, &QHeaderView::sectionMoved, this, &TableView::sectionMoved);

	// do this initialization here after the model knows about
	// the number of columns. Otherwise the resize column method
	// won't work
	m->header->init(headers, headerState, sortorder());
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

void TableView::sectionResized(int logicalIndex, int oldSize, int newSize)
{
	Q_UNUSED(logicalIndex)
	Q_UNUSED(oldSize)
	Q_UNUSED(newSize)

	if(!this->isVisible()){
		return;
	}

	saveColumnHeaderState(m->header->saveState());
}

void TableView::sectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
	Q_UNUSED(logicalIndex)
	Q_UNUSED(oldVisualIndex)
	Q_UNUSED(newVisualIndex)

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
