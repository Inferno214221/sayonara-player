
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
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <QFontMetrics>

namespace Algorithm=Util::Algorithm;
using namespace Library;
using Parent=Gui::WidgetTemplate<QHeaderView>;

struct HeaderView::Private
{
	ColumnHeaderList	columns;
	QAction*			action_resize=nullptr;
};

HeaderView::HeaderView(Qt::Orientation orientation, QWidget* parent) :
	Parent(orientation, parent)
{
	m = Pimpl::make<Private>();

	m->action_resize = new QAction(resize_text(), this);

	addAction(m->action_resize);
	connect(m->action_resize, &QAction::triggered, this, &HeaderView::action_resize_triggered);

	this->setSectionsClickable(true);
	this->setHighlightSections(false);
	this->setContextMenuPolicy(Qt::ActionsContextMenu);
}

HeaderView::~HeaderView() = default;

void HeaderView::init_header_action(ColumnHeaderPtr header, bool is_shown)
{
	QAction* action = header->action();
	action->setChecked(is_shown);
	connect(action, &QAction::toggled, this, &HeaderView::action_triggered);

	this->insertAction(this->actions().last(), action);
}

QString HeaderView::resize_text() const
{
	return tr("Resize columns");
}

void HeaderView::set_columns(const ColumnHeaderList& columns, const BoolList& shown_actions, Library::SortOrder sorting)
{
	m->columns = columns;

	for(int i=0; i<m->columns.size(); i++)
	{
		ColumnHeaderPtr section = m->columns[i];

		{ // sorting
			if(sorting == section->sortorder_asc()) {
				this->setSortIndicator(i, Qt::AscendingOrder);
			}

			else if(sorting == section->sortorder_desc()) {
				this->setSortIndicator(i, Qt::DescendingOrder);
			}
		}

		bool is_visible = true;
		{ // resize mode
			if(Util::between(i, shown_actions)) {
				is_visible = shown_actions[i];
			}

			if(is_visible && !section->stretchable())
			{
				this->resizeSection(i, section->preferred_size());
			}
		}

		init_header_action(section, is_visible);
	}

	refresh_active_columns();
}

BoolList HeaderView::refresh_active_columns()
{
	BoolList lst;

	for(int i=0; i<m->columns.count(); i++)
	{
		ColumnHeaderPtr section = m->columns[i];

		bool is_visible = section->is_action_checked();
		this->setSectionHidden(i, !is_visible);
		lst.push_back(is_visible);
	}

	return lst;
}

BoolList HeaderView::shown_columns() const
{
	BoolList ret;
	for(int i=0; i<m->columns.count(); i++)
	{
		ret << (this->isSectionHidden(i) == false);
	}

	return ret;
}

ColumnHeaderPtr HeaderView::column(int idx)
{
	if(!Util::between(idx, m->columns)){
		return nullptr;
	}

	return m->columns[idx];
}


void HeaderView::language_changed()
{
	for(ColumnHeaderPtr header : Algorithm::AsConst(m->columns))
	{
		header->retranslate();
	}

	if(m->action_resize){
		m->action_resize->setText(resize_text());
	}
}

void HeaderView::action_triggered(bool b)
{
	Q_UNUSED(b)

	refresh_active_columns();
	emit sig_columns_changed();
}


void HeaderView::action_resize_triggered()
{
	double scale_factor;
	{	// calculate scale factor of stretchable columns
		int space_needed = 0;
		int free_space = this->width();

		for(int i=0; i<m->columns.count(); i++)
		{
			auto ch = m->columns[i];

			int sz = ch->default_size();

			if( !this->isSectionHidden(i) )
			{
				if(ch->stretchable()) {
					space_needed += sz;
				}

				else {
					this->resizeSection(i, sz);
					free_space -= sz;
				}
			}
		}

		scale_factor = std::max(free_space * 1.0 / space_needed, 1.0);
	}

	{ // resize stretchable sections
		for(int i=0; i<m->columns.count(); i++)
		{
			auto ch = m->columns[i];

			if(ch->stretchable() && !this->isSectionHidden(i))
			{
				int sz = static_cast<int>(ch->default_size() * scale_factor);
				this->resizeSection(i, sz);
			}
		}
	}
}


int HeaderView::calc_header_width() const
{
	int header_width = 0;
	for(int i=0; i< m->columns.count(); i++)
	{
		header_width += this->sectionSize(i);
	}

	return header_width;
}

void HeaderView::resizeEvent(QResizeEvent* e)
{
	int last_visible_index = 0;
	int old_last_visible_size = 50;

	QList<int> stretchable_indices;
	for(int i=0; i<m->columns.size(); i++)
	{
		if(this->isSectionHidden(i)) {
			continue;
		}

		last_visible_index = i;
		old_last_visible_size = this->sectionSize(i);

		if(m->columns[i]->stretchable())
		{
			stretchable_indices << i;
		}
	}

	Parent::resizeEvent(e);

	{ // resize all stretchable sections according to the space there is left in the last column
		int cur_last_visible_size = this->sectionSize(last_visible_index);
		if(cur_last_visible_size <= 0 || stretchable_indices.isEmpty()){
			return;
		}

		// space we have from last index
		int space = std::max(0, cur_last_visible_size - old_last_visible_size);

		// the header view is too big and we are shrinking the window -> Shrink the header view, too
		int x_difference = e->size().width() - e->oldSize().width();
		if(calc_header_width() > this->width() && x_difference < 0){
			space += x_difference;
		}

		int space_per_item = (space) / stretchable_indices.size();
		int remainder = (space) % stretchable_indices.size();

		if(space_per_item == 0){
			return;
		}

		for(int index : stretchable_indices)
		{
			int default_size = m->columns[index]->default_size();

			int new_size = this->sectionSize(index) + space_per_item + std::min(remainder, 1);
			    new_size = std::max(default_size, new_size);

			remainder = std::max(0, remainder - 1);

			this->resizeSection(index, new_size);
		}

		this->resizeSection(last_visible_index, m->columns[last_visible_index]->default_size());
		this->setSectionResizeMode(last_visible_index, QHeaderView::Fixed);
	}
}


QSize HeaderView::sizeHint() const
{
	int height = std::max(this->fontMetrics().height() + 10, 20);

	QSize size = QHeaderView::sizeHint();
	size.setHeight(height);

	return size;
}
