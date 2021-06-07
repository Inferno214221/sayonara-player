
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

#include "LibraryHeaderView.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/GuiUtils.h"

#include <QAction>
#include <QFontMetrics>
#include <QPair>

using Library::ColumnHeaderPtr;
using Library::HeaderView;

using ColumnActionPair = QPair<ColumnHeaderPtr, QAction*>;
using ColumnActionPairList = QList<ColumnActionPair>;

namespace
{
	int columnWidth(const Library::ColumnHeaderPtr& columnHeader, QWidget* widget)
	{
		return std::max(columnHeader->defaultSize(),
		                Gui::Util::textWidth(widget, columnHeader->title() + "MMM"));
	}

	void restoreActions(const ColumnActionPairList& columns, const QHeaderView* headerView)
	{
		auto i=0;
		for(auto& [columnHeader, action] : columns)
		{
			action->setCheckable(columnHeader->isSwitchable());
			action->setChecked(!headerView->isSectionHidden(i));
			i++;
		}
	}
}

struct HeaderView::Private
{
	ColumnActionPairList columns;
	QAction* actionResize;
	QAction* actionAutoResize;

	QByteArray initialState;
	bool isInitialized {false};

	Private(HeaderView* parent) :
		actionResize {new QAction(parent)},
		actionAutoResize {new QAction(parent)}
	{
		actionAutoResize->setCheckable(parent);
	}
};

HeaderView::HeaderView(Qt::Orientation orientation, QWidget* parent) :
	Gui::HeaderView(orientation, parent)
{
	m = Pimpl::make<Private>(this);

	connect(m->actionResize, &QAction::triggered, this, &HeaderView::actionResizeTriggered);
	connect(m->actionAutoResize, &QAction::triggered, this, &HeaderView::actionAutoResizeTriggered);
	connect(this, &QHeaderView::sectionDoubleClicked, this, [=](int /*logicalIndex*/) {
		this->resizeColumnsAutomatically();
	});
}

HeaderView::~HeaderView() = default;

void HeaderView::init(const ColumnHeaderList& columnHeaderList, const QByteArray& state, Library::SortOrder sortOrder,
                      bool autoResizeState)
{
	m->initialState = state;

	for(auto i = 0; i < columnHeaderList.size(); i++)
	{
		const auto columnHeader = columnHeaderList[i];

		// action
		auto* action = new QAction(columnHeader->title());
		action->setCheckable(columnHeader->isSwitchable());
		action->setChecked(!isSectionHidden(i));

		connect(action, &QAction::toggled, this, &HeaderView::actionTriggered);
		this->addAction(action);

		// sorting
		if(sortOrder == columnHeader->sortorder(Qt::AscendingOrder))
		{
			this->setSortIndicator(i, Qt::AscendingOrder);
		}

		else if(sortOrder == columnHeader->sortorder(Qt::DescendingOrder))
		{
			this->setSortIndicator(i, Qt::DescendingOrder);
		}

		m->columns << ColumnActionPair(columnHeader, action);
	}

	auto* sep = new QAction();
	sep->setSeparator(true);

	this->addAction(sep);
	this->addAction(m->actionResize);
	this->addAction(m->actionAutoResize);

	m->actionAutoResize->setChecked(autoResizeState);
}

void HeaderView::initializeView()
{
	if(m->initialState.isEmpty())
	{
		this->resizeColumnsAutomatically();
	}

	else
	{
		this->restoreState(m->initialState);
		m->initialState.clear();

		restoreActions(m->columns, this);
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

Library::SortOrder HeaderView::sortorder(int index, Qt::SortOrder sortOrder)
{
	if(Util::between(index, m->columns))
	{
		const auto& columnHeader = m->columns[index].first;
		return columnHeader->sortorder(sortOrder);
	}

	return Library::SortOrder::NoSorting;
}

QString HeaderView::columnText(int index) const
{
	return (Util::between(index, m->columns))
	       ? m->columns[index].first->title()
	       : QString();
}

void HeaderView::actionTriggered(bool b)
{
	auto* action = dynamic_cast<QAction*>(sender());
	const auto index = this->actions().indexOf(action);
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

void HeaderView::resizeColumnsAutomatically()
{
	double scaleFactor;
	{    // calculate scale factor of stretchable columns
		auto spaceNeeded = 0;
		auto freeSpace = this->width();

		for(auto i = 0; i < m->columns.size(); i++)
		{
			if(this->isSectionHidden(i))
			{
				continue;
			}

			const auto& columnHeader = m->columns[i].first;
			const auto size = columnWidth(columnHeader, this);

			if(columnHeader->isStretchable())
			{
				spaceNeeded += size;
			}

			else
			{
				this->resizeSection(i, size);
				freeSpace -= size;
			}
		}

		scaleFactor = std::max(freeSpace * 1.0 / spaceNeeded, 1.0);
	}

	{ // resize stretchable sections
		for(auto i = 0; i < m->columns.size(); i++)
		{
			const auto& columnHeader = m->columns[i].first;
			if(columnHeader->isStretchable() && !this->isSectionHidden(i))
			{
				const auto size = columnWidth(columnHeader, this);
				this->resizeSection(i, static_cast<int>(size * scaleFactor));
			}
		}
	}
}

QSize HeaderView::sizeHint() const
{
	return QSize(0, (fontMetrics().height() * 3) / 2);
}

void HeaderView::reloadColumnTexts()
{
	for(auto i = 0; i < m->columns.size(); i++)
	{
		auto* action = m->columns[i].second;
		action->setText(columnText(i));
	}
}

void HeaderView::languageChanged()
{
	m->actionResize->setText(tr("Resize columns"));
	m->actionAutoResize->setText(tr("Resize columns automatically"));

	reloadColumnTexts();
}

void HeaderView::showEvent(QShowEvent* e)
{
	Gui::HeaderView::showEvent(e);

	if(!m->isInitialized)
	{
		initializeView();
	}
}

void HeaderView::resizeEvent(QResizeEvent* e)
{
	Gui::HeaderView::resizeEvent(e);

	if(m && m->actionAutoResize->isChecked())
	{
		this->resizeColumnsAutomatically();
	}
}
