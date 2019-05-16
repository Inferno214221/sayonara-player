
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

#include "HeaderView.h"
#include "Utils/globals.h"
#include "Utils/Utils.h"

#include <QFontMetrics>

#include <algorithm>

using namespace Library;

struct HeaderView::Private
{
	ColumnHeaderList	column_headers;
};

HeaderView::HeaderView(Qt::Orientation orientation, QWidget* parent) :
	WidgetTemplate<QHeaderView>(orientation, parent)
{
	m = Pimpl::make<Private>();

	this->setSectionsClickable(true);
	this->setStretchLastSection(true);
	this->setHighlightSections(false);
	this->setContextMenuPolicy(Qt::ActionsContextMenu);
}

HeaderView::~HeaderView() {}

void HeaderView::init_header_action(ColumnHeaderPtr header, bool is_shown)
{
	QAction* action = header->action();
	action->setChecked(is_shown);
	connect(action, &QAction::toggled, this, &HeaderView::action_triggered);

	this->addAction(action);
}

void HeaderView::set_column_headers(const ColumnHeaderList& column_headers, const BoolList& shown_actions, Library::SortOrder sorting)
{
	m->column_headers = column_headers;

	for(int i=0; i<m->column_headers.count(); i++)
	{
		ColumnHeaderPtr section = m->column_headers[i];

		if(section->sortorder_asc() == sorting) {
			this->setSortIndicator(i, Qt::AscendingOrder);
		}

		else if(section->sortorder_desc() == sorting) {
			this->setSortIndicator(i, Qt::DescendingOrder);
		}

		bool is_shown = true;
		if(between(i, shown_actions)){
			is_shown = shown_actions[i];
		}

		if(is_shown){
			this->resizeSection(i, section->preferred_size());
		}

		init_header_action(section, is_shown);
	}

	refresh_active_columns();

	for(int i=0; i<m->column_headers.count()-1; i++){
		this->setSectionResizeMode(1, QHeaderView::Interactive);
	}

	this->setSectionResizeMode(m->column_headers.count()-1, QHeaderView::Stretch);
}

BoolList HeaderView::refresh_active_columns()
{
	BoolList lst;

	for(int i=0; i<m->column_headers.count(); i++)
	{
		ColumnHeaderPtr section = m->column_headers[i];

		this->setSectionHidden(i, section->is_hidden());
		lst.push_back(section->is_visible());
	}

	return lst;
}

BoolList HeaderView::shown_columns() const
{
	BoolList lst;

	for(ColumnHeaderPtr section : m->column_headers)
	{
		lst.push_back(section->is_visible());
	}

	return lst;
}

ColumnHeaderPtr HeaderView::column_header(int idx)
{
	if(!between(idx, m->column_headers)){
		return nullptr;
	}

	return m->column_headers[idx];
}


void HeaderView::language_changed()
{
	for(ColumnHeaderPtr header : Util::AsConst(m->column_headers))
	{
		header->retranslate();
	}
}

QSize HeaderView::sizeHint() const
{
	QSize size = QHeaderView::sizeHint();

	int height = std::max(this->fontMetrics().height() + 10, 20);

	size.setHeight(height);

	return size;
}

void HeaderView::action_triggered(bool b)
{
	BoolList shown_cols = refresh_active_columns();

	emit sig_columns_changed();

	Q_UNUSED(b)
	Q_UNUSED(shown_cols)
}
