
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

#include "ColumnHeader.h"
#include "HeaderView.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/GuiUtils.h"

#include <QFontMetrics>
#include <QPair>

using Library::ColumnHeaderPtr;
using Library::HeaderView;
using Parent=Gui::WidgetTemplate<QHeaderView>;

using ColumnActionPair = QPair<ColumnHeaderPtr, QAction*>;
using ColumnActionPairList = QList<ColumnActionPair>;

struct HeaderView::Private
{
	ColumnActionPairList columns;
	QAction* actionResize=nullptr;
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
	if(!state.isEmpty()) {
		this->restoreState(state);
	}

	else {
		this->actionResizeTriggered();
	}

	for(int i=0; i<columns.size(); i++)
	{
		ColumnHeaderPtr section = columns[i];

		{ // action
			auto* action = new QAction(section->title());
			action->setCheckable(true);
			action->setChecked( isSectionHidden(i) == false );

			connect(action, &QAction::toggled, this, &HeaderView::actionTriggered);

			insertAction(this->actions().last(), action);
			m->columns << ColumnActionPair(section, action);
		}

		{ // sorting
			if(sorting == section->sortorder(Qt::AscendingOrder)) {
				this->setSortIndicator(i, Qt::AscendingOrder);
			}

			else if(sorting == section->sortorder(Qt::DescendingOrder)) {
				this->setSortIndicator(i, Qt::DescendingOrder);
			}
		}
	}

	// sort columns by index
	Util::Algorithm::sort(m->columns, [](ColumnActionPair p1, ColumnActionPair p2){
		return (p1.first->columnIndex() < p2.first->columnIndex());
	});
}

Library::SortOrder HeaderView::sortorder(int index, Qt::SortOrder sortorder)
{
	if(Util::between(index, m->columns))
	{
		ColumnHeaderPtr section = m->columns[index].first;
		return section->sortorder(sortorder);
	}

	return Library::SortOrder::NoSorting;
}

QString HeaderView::columnText(int index) const
{
	if(!Util::between(index, m->columns)){
		return QString();
	}

	return m->columns[index].first->title();
}

void HeaderView::actionTriggered(bool b)
{
	auto* action = static_cast<QAction*>(sender());
	int index = this->actions().indexOf(action);
	if(index >= 0) {
		this->setSectionHidden(index, !b);
	}

	actionResizeTriggered();
}

static int columnWidth(Library::ColumnHeaderPtr section, const QFontMetrics& fm)
{
	return std::max
	(
		section->defaultSize(),
		Gui::Util::textWidth(fm, section->title() + "MMM")
	);
}

void HeaderView::actionResizeTriggered()
{
	const QFontMetrics fm = this->fontMetrics();

	double scaleFactor;
	{	// calculate scale factor of stretchable columns
		int spaceNeeded = 0;
		int freeSpace = this->width();

		int i=0;
		for(auto it=m->columns.begin(); it != m->columns.end(); it++, i++)
		{
			if(this->isSectionHidden(i)){
				continue;
			}

			ColumnHeaderPtr section = it->first;
			const int size = columnWidth(section, fm);

			if(!section->stretchable())
			{
				this->resizeSection(i, size);
				freeSpace -= size;
			}

			else {
				spaceNeeded += size;
			}
		}

		scaleFactor = std::max(freeSpace * 1.0 / spaceNeeded, 1.0);
	}

	{ // resize stretchable sections
		int i=0;
		for(auto it=m->columns.begin(); it != m->columns.end(); it++, i++)
		{			
			ColumnHeaderPtr section = it->first;
			if(section->stretchable() && !this->isSectionHidden(i))
			{
				const int size = columnWidth(section, fm);
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

void HeaderView::languageChanged()
{
	const QFontMetrics fm = this->fontMetrics();

	int i=0;
	for(auto it=m->columns.begin(); it != m->columns.end(); it++, i++)
	{
		ColumnHeaderPtr section = it->first;
		QAction* action = it->second;

		QString text = section->title();
		action->setText(text);

		int headerTextWidth = Gui::Util::textWidth(fm, text + "MMM");
		if(this->sectionSize(i) < headerTextWidth)
		{
			this->resizeSection(i, headerTextWidth);
		}
	}

	if(m->actionResize) {
		m->actionResize->setText(resizeText());
	}
}

QSize HeaderView::sizeHint() const
{
	QSize size = QHeaderView::sizeHint();

	int height = std::max(this->fontMetrics().height() + 10, 20);
	size.setHeight(height);

	return size;
}
