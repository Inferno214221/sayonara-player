
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
#include <QPair>

using Library::ColumnHeaderPtr;
using Library::HeaderView;
using Parent = Gui::WidgetTemplate<QHeaderView>;

using ColumnActionPair = QPair<ColumnHeaderPtr, QAction*>;
using ColumnActionPairList = QList<ColumnActionPair>;

struct HeaderView::Private
{
	ColumnActionPairList columns;
	QAction* actionResize = nullptr;
	QAction* actionAutoResize = nullptr;

	QByteArray initialState;
	bool isInitialized;

	Private(HeaderView* parent) :
		isInitialized(false)
	{
		actionResize = new QAction(parent);
		actionAutoResize = new QAction(parent);
		actionAutoResize->setCheckable(parent);
	}
};

HeaderView::HeaderView(Qt::Orientation orientation, QWidget* parent) :
	Parent(orientation, parent)
{
	m = Pimpl::make<Private>(this);

	connect(m->actionResize, &QAction::triggered, this, &HeaderView::actionResizeTriggered);
	connect(m->actionAutoResize, &QAction::triggered, this, &HeaderView::actionAutoResizeTriggered);
	connect(this, &QHeaderView::sectionDoubleClicked, this, [this](int /*logicalIndex*/) {
		this->resizeColumnsAutomatically();
	});
}

HeaderView::~HeaderView() = default;

void HeaderView::init(const ColumnHeaderList& columns, const QByteArray& state, Library::SortOrder sorting,
                      bool autoResizeState)
{
	m->initialState = state;

	for(int i = 0; i < columns.size(); i++)
	{
		ColumnHeaderPtr section = columns[i];

		// action
		auto* action = new QAction(section->title());
		action->setCheckable(section->isSwitchable());
		action->setChecked(!isSectionHidden(i));

		connect(action, &QAction::toggled, this, &HeaderView::actionTriggered);
		this->addAction(action);

		// sorting
		if(sorting == section->sortorder(Qt::AscendingOrder))
		{
			this->setSortIndicator(i, Qt::AscendingOrder);
		}

		else if(sorting == section->sortorder(Qt::DescendingOrder))
		{
			this->setSortIndicator(i, Qt::DescendingOrder);
		}

		m->columns << ColumnActionPair(section, action);
	}

	auto* sep = new QAction();
	sep->setSeparator(true);

	this->addAction(sep);
	this->addAction(m->actionResize);
	this->addAction(m->actionAutoResize);

	m->actionAutoResize->setChecked(autoResizeState);
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
	if(!Util::between(index, m->columns))
	{
		return QString();
	}

	return m->columns[index].first->title();
}

void HeaderView::reloadColumnTexts()
{
	for(int i = 0; i < m->columns.size(); i++)
	{
		QAction* action = m->columns[i].second;
		action->setText(columnText(i));
	}
}

void HeaderView::actionTriggered(bool b)
{
	auto* action = dynamic_cast<QAction*>(sender());
	int index = this->actions().indexOf(action);
	if(index >= 0)
	{
		this->setSectionHidden(index, !b);
	}

	actionResizeTriggered();
}

void HeaderView::actionResizeTriggered()
{
	resizeColumnsAutomatically();
}

void HeaderView::actionAutoResizeTriggered(bool b)
{
	emit sigAutoResizeToggled(b);
}

static int columnWidth(Library::ColumnHeaderPtr section, QWidget* widget)
{
	return std::max
		(
			section->defaultSize(),
			Gui::Util::textWidth(widget, section->title() + "MMM")
		);
}

void HeaderView::resizeColumnsAutomatically()
{
	double scaleFactor;
	{    // calculate scale factor of stretchable columns
		int spaceNeeded = 0;
		int freeSpace = this->width();

		for(int i = 0; i < m->columns.size(); i++)
		{
			if(this->isSectionHidden(i))
			{
				continue;
			}

			ColumnHeaderPtr section = m->columns[i].first;
			int size = columnWidth(section, this);

			if(!section->isStretchable())
			{
				this->resizeSection(i, size);
				freeSpace -= size;
			}

			else
			{
				spaceNeeded += size;
			}
		}

		scaleFactor = std::max(freeSpace * 1.0 / spaceNeeded, 1.0);
	}

	{ // resize stretchable sections
		for(int i = 0; i < m->columns.size(); i++)
		{
			ColumnHeaderPtr section = m->columns[i].first;
			if(section->isStretchable() && !this->isSectionHidden(i))
			{
				int size = columnWidth(section, this);
				this->resizeSection(i, int(size * scaleFactor));
			}
		}
	}
}

QSize HeaderView::sizeHint() const
{
	return QSize(0, (fontMetrics().height() * 3) / 2);
}

void HeaderView::languageChanged()
{
	m->actionResize->setText(tr("Resize columns"));
	m->actionAutoResize->setText(tr("Resize columns automatically"));
}

void HeaderView::showEvent(QShowEvent* e)
{
	Parent::showEvent(e);

	if(m->isInitialized)
	{
		return;
	}

	if(m->initialState.isEmpty())
	{
		this->resizeColumnsAutomatically();
	}

	else
	{
		this->restoreState(m->initialState);
	}

	this->setMinimumSectionSize(25);
	this->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
	this->setCascadingSectionResizes(false);
	this->setSectionsClickable(true);
	this->setSectionsMovable(true);
	this->setHighlightSections(true);
	this->setContextMenuPolicy(Qt::ActionsContextMenu);
	this->setTextElideMode(Qt::TextElideMode::ElideRight);
	this->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
	this->setStretchLastSection(true);
	this->setSortIndicatorShown(true);

	m->isInitialized = true;
}

void HeaderView::resizeEvent(QResizeEvent* e)
{
	Parent::resizeEvent(e);

	if(m && m->actionAutoResize->isChecked())
	{
		this->resizeColumnsAutomatically();
	}
}
