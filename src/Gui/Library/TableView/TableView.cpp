/* LibraryTableView.cpp */

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

#include "TableView.h"

#include "Gui/Library/Header/ColumnHeader.h"
#include "Gui/Library/Header/LibraryHeaderView.h"
#include "Gui/Library/ItemModel.h"
#include "Gui/Utils/GuiUtils.h"

#include "Utils/Algorithm.h"
#include "Utils/Set.h"

using namespace Library;

struct TableView::Private
{
	HeaderView* header;

	Private(TableView* tableView) :
		header {new HeaderView(Qt::Horizontal, tableView)} {}
};

TableView::TableView(QWidget* parent) :
	Library::ItemView(parent),
	m {Pimpl::make<Private>(this)}
{
	setHorizontalHeader(m->header);
}

TableView::~TableView() = default;

void TableView::init(AbstractLibrary* library)
{
	verticalHeader()->setDefaultSectionSize(Gui::Util::viewRowHeight(fontMetrics()));

	initView(library);

	auto headers = columnHeaders();
	Util::Algorithm::sort(headers, [](const auto& p1, const auto& p2) {
		return (p1->columnIndex() < p2->columnIndex());
	});

	m->header->init(headers, columnHeaderState(), sortorder(), autoResizeState());

	connect(m->header, &QHeaderView::sectionCountChanged, this, &TableView::headerColumnsChanged);
	connect(m->header, &QHeaderView::sectionResized, this, &TableView::sectionResized);
	connect(m->header, &QHeaderView::sectionMoved, this, &TableView::sectionMoved);
	connect(m->header, &QHeaderView::sortIndicatorChanged, this, &TableView::sortorderChanged);
	connect(m->header, &HeaderView::sigAutoResizeToggled, this, &TableView::autoResizeTriggered);

	languageChanged();
}

void TableView::headerColumnsChanged(int /*oldCount*/, int /*newCount*/)
{
	const auto selectedIndexes = selectedItems();
	for(const auto index: selectedIndexes)
	{
		selectRow(index);
	}

	setupColumnNames();

	if(isVisible())
	{
		saveColumnHeaderState(m->header->saveState());
	}
}

void TableView::sortorderChanged(const int index, const Qt::SortOrder qtSortorder)
{
	applySortorder(m->header->sortorder(index, qtSortorder));
	if(isVisible())
	{
		saveColumnHeaderState(m->header->saveState());
	}
}

void TableView::sectionResized(int /*logicalIndex*/, int /*oldSize*/, int /*newSize*/)
{
	if(isVisible())
	{
		saveColumnHeaderState(m->header->saveState());
	}
}

void TableView::sectionMoved(int /*logicalIndex*/, int /*oldVisualIndex*/, int /*newVisualIndex*/)
{
	setupColumnNames();

	if(isVisible())
	{
		saveColumnHeaderState(m->header->saveState());
	}
}

void TableView::autoResizeTriggered(bool b)
{
	saveAutoResizeState(b);
	if(b)
	{
		m->header->resizeColumnsAutomatically();
	}
}

void TableView::setupColumnNames()
{
	const auto count = columnHeaders().count();
	for(int i = 0; i < count; i++)
	{
		const auto text = m->header->columnText(i);
		model()->setHeaderData(i, Qt::Horizontal, text, Qt::DisplayRole);
	}

	m->header->reloadColumnTexts();
}

void TableView::languageChanged()
{
	setupColumnNames();
}

int TableView::mapModelIndexToIndex(const QModelIndex& idx) const { return idx.row(); }

ModelIndexRange TableView::mapIndexToModelIndexes(int idx) const
{
	return {
		itemModel()->index(idx, 0),
		itemModel()->index(idx, itemModel()->columnCount() - 1)
	};
}
