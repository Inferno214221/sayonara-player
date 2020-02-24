
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

#include "HeaderView.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/GuiUtils.h"

#include <QFontMetrics>

using Library::HeaderView;
using Parent=Gui::WidgetTemplate<QHeaderView>;
namespace Algorithm=Util::Algorithm;

struct HeaderView::Private
{
	ColumnHeaderList	columns;
	QAction*			actionResize=nullptr;
};

HeaderView::HeaderView(Qt::Orientation orientation, QWidget* parent) :
	Parent(orientation, parent)
{
	m = Pimpl::make<Private>();

	m->actionResize = new QAction(resizeText(), this);

	addAction(m->actionResize);
	connect(m->actionResize, &QAction::triggered, this, &HeaderView::actionResizeTriggered);

	this->setSectionsClickable(true);
	this->setSectionsMovable(true);
	this->setHighlightSections(false);
	this->setContextMenuPolicy(Qt::ActionsContextMenu);
	this->setTextElideMode(Qt::TextElideMode::ElideRight);
	this->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
}

HeaderView::~HeaderView() = default;

QString HeaderView::resizeText() const
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
		this->actionResizeTriggered();
	}

	for(int i=0; i<m->columns.size(); i++)
	{
		ColumnHeaderPtr section = m->columns[i];

		{ // sorting
			if(sorting == section->sortorderAscending()) {
				this->setSortIndicator(i, Qt::AscendingOrder);
			}

			else if(sorting == section->sortorderDescending()) {
				this->setSortIndicator(i, Qt::DescendingOrder);
			}
		}

		QAction* action = section->action();
		action->setChecked( !isSectionHidden(i) );
		connect(action, &QAction::toggled, this, &HeaderView::actionTriggered);
		insertAction(this->actions().last(), action);
	}
}

Library::SortOrder HeaderView::switchSortorder(int column_index)
{
	if(Util::between(column_index, m->columns))
	{
		Qt::SortOrder asc_desc = this->sortIndicatorOrder();

		ColumnHeaderPtr section = m->columns[column_index];
		if(asc_desc == Qt::AscendingOrder) {
			return section->sortorderAscending();
		}

		else {
			return section->sortorderDescending();
		}
	}

	return Library::SortOrder::NoSorting;
}


Library::ColumnHeaderPtr HeaderView::column(int column_index)
{
	if(!Util::between(column_index, m->columns)){
		return nullptr;
	}

	return m->columns[column_index];
}


void HeaderView::languageChanged()
{
	const QFontMetrics fm = this->fontMetrics();

	int i=0;
	for(auto it=m->columns.begin(); it != m->columns.end(); it++, i++)
	{
		ColumnHeaderPtr header = *it;

		header->retranslate();

		int header_text_width = Gui::Util::textWidth(fm, header->title() + "MMM");
		if(this->sectionSize(i) < header_text_width)
		{
			this->resizeSection(i, header_text_width);
		}
	}

	if(m->actionResize) {
		m->actionResize->setText(resizeText());
	}
}

void HeaderView::actionTriggered(bool b)
{
	Q_UNUSED(b)

	for(int i=0; i<m->columns.count(); i++)
	{
		ColumnHeaderPtr section = m->columns[i];

		bool isVisible = section->isActionChecked();
		this->setSectionHidden(i, !isVisible);
	}

	actionResizeTriggered();

	emit sigColumnsChanged();
}

static int columnWidth(Library::ColumnHeaderPtr ch, const QFontMetrics& fm)
{
	return std::max
	(
		ch->defaultSize(),
		Gui::Util::textWidth(fm, ch->title() + "MMM")
	);
}

void HeaderView::actionResizeTriggered()
{
	const QFontMetrics fm = this->fontMetrics();

	double scaleFactor;
	{	// calculate scale factor of stretchable columns
		int spaceNeeded = 0;
		int freeSpace = this->width();

		for(int i=0; i<m->columns.count(); i++)
		{
			ColumnHeaderPtr ch = m->columns[i];

			if( !this->isSectionHidden(i) )
			{
				const int size = columnWidth(ch, fm);

				if(!ch->stretchable())
				{
					this->resizeSection(i, size);
					freeSpace -= size;
				}

				else {
					spaceNeeded += size;
				}
			}
		}

		scaleFactor = std::max(freeSpace * 1.0 / spaceNeeded, 1.0);
	}

	{ // resize stretchable sections
		for(int i=0; i<m->columns.count(); i++)
		{
			ColumnHeaderPtr ch = m->columns[i];

			if(ch->stretchable() && !this->isSectionHidden(i))
			{
				const int size = columnWidth(ch, fm);
				this->resizeSection(i, int(size * scaleFactor));
			}
		}
	}
}

int HeaderView::calcHeaderWidth() const
{
	int headerWidth = 0;
	for(int i=0; i< m->columns.count(); i++)
	{
		headerWidth += this->sectionSize(i);
	}

	return headerWidth;
}

QSize HeaderView::sizeHint() const
{
	int height = std::max(this->fontMetrics().height() + 10, 20);

	QSize size = QHeaderView::sizeHint();
	size.setHeight(height);

	return size;
}
