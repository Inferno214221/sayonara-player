
/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "Gui/Utils/GuiUtils.h"

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
	this->setSectionsMovable(true);
	this->setHighlightSections(false);
	this->setContextMenuPolicy(Qt::ActionsContextMenu);
	this->setTextElideMode(Qt::TextElideMode::ElideRight);
	this->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
}

HeaderView::~HeaderView() = default;

QString HeaderView::resize_text() const
{
	return tr("Resize columns");
}

void HeaderView::init(const ColumnHeaderList& columns, const QByteArray& state, Library::SortOrder sorting)
{
	m->columns = columns;

	if(!state.isEmpty()) {
		this->restoreState(state);
	}

	else {
		this->action_resize_triggered();
	}

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

		QAction* action = section->action();
		action->setChecked( !isSectionHidden(i) );
		connect(action, &QAction::toggled, this, &HeaderView::action_triggered);
		insertAction(this->actions().last(), action);
	}
}

Library::SortOrder HeaderView::switch_sortorder(int column_index)
{
	if(Util::between(column_index, m->columns))
	{
		Qt::SortOrder asc_desc = this->sortIndicatorOrder();

		ColumnHeaderPtr section = m->columns[column_index];
		if(asc_desc == Qt::AscendingOrder) {
			return section->sortorder_asc();
		}

		else {
			return section->sortorder_desc();
		}
	}

	return Library::SortOrder::NoSorting;
}


ColumnHeaderPtr HeaderView::column(int column_index)
{
	if(!Util::between(column_index, m->columns)){
		return nullptr;
	}

	return m->columns[column_index];
}


void HeaderView::language_changed()
{
	const QFontMetrics fm = this->fontMetrics();

	int i=0;
	for(auto it=m->columns.begin(); it != m->columns.end(); it++, i++)
	{
		ColumnHeaderPtr header = *it;

		header->retranslate();

		int header_text_width = Gui::Util::text_width(fm, header->title() + "MMM");
		if(this->sectionSize(i) < header_text_width)
		{
			this->resizeSection(i, header_text_width);
		}
	}

	if(m->action_resize) {
		m->action_resize->setText(resize_text());
	}
}

void HeaderView::action_triggered(bool b)
{
	Q_UNUSED(b)

	for(int i=0; i<m->columns.count(); i++)
	{
		ColumnHeaderPtr section = m->columns[i];

		bool is_visible = section->is_action_checked();
		this->setSectionHidden(i, !is_visible);
	}

	action_resize_triggered();

	emit sig_columns_changed();
}

static int column_width(ColumnHeaderPtr ch, const QFontMetrics& fm)
{
	return std::max
	(
		ch->default_size(),
		Gui::Util::text_width(fm, ch->title() + "MMM")
	);
}

void HeaderView::action_resize_triggered()
{
	const QFontMetrics fm = this->fontMetrics();

	double scale_factor;
	{	// calculate scale factor of stretchable columns
		int space_needed = 0;
		int free_space = this->width();

		for(int i=0; i<m->columns.count(); i++)
		{
			ColumnHeaderPtr ch = m->columns[i];

			if( !this->isSectionHidden(i) )
			{
				const int size = column_width(ch, fm);

				if(!ch->stretchable())
				{
					this->resizeSection(i, size);
					free_space -= size;
				}

				else {
					space_needed += size;
				}
			}
		}

		scale_factor = std::max(free_space * 1.0 / space_needed, 1.0);
	}

	{ // resize stretchable sections
		for(int i=0; i<m->columns.count(); i++)
		{
			ColumnHeaderPtr ch = m->columns[i];

			if(ch->stretchable() && !this->isSectionHidden(i))
			{
				const int size = column_width(ch, fm);
				this->resizeSection(i, int(size * scale_factor));
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
	Parent::resizeEvent(e);
	action_resize_triggered();
}

QSize HeaderView::sizeHint() const
{
	int height = std::max(this->fontMetrics().height() + 10, 20);

	QSize size = QHeaderView::sizeHint();
	size.setHeight(height);

	return size;
}
