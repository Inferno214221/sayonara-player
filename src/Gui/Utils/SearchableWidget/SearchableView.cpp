/* SearchableView.cpp */

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

#include "SearchableView.h"
#include "SearchableModel.h"
#include "MiniSearcher.h"
#include "Utils/Algorithm.h"

#include "Utils/Library/SearchMode.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QAbstractItemModel>
#include <QKeyEvent>
#include <QListView>
#include <QMap>
#include <QScrollBar>
#include <QString>

struct MiniSearcherViewConnector::Private
{
	QMap<QChar, QString> triggerMap;
	QString currentSearchstring;

	Gui::MiniSearcher* miniSearcher {nullptr};
	SearchView* searchView;

	explicit Private(SearchView* searchView) :
		searchView {searchView} {}
};

MiniSearcherViewConnector::MiniSearcherViewConnector(SearchView* searchView, QObject* parent) :
	QObject(parent),
	m {Pimpl::make<Private>(searchView)} {}

MiniSearcherViewConnector::~MiniSearcherViewConnector() = default;

void MiniSearcherViewConnector::init()
{
	if(m->miniSearcher)
	{
		return;
	}
	
	m->miniSearcher = new Gui::MiniSearcher(m->searchView);
	m->miniSearcher->setExtraTriggers(m->triggerMap);

	connect(m->miniSearcher, &Gui::MiniSearcher::sigTextChanged, this, &MiniSearcherViewConnector::lineEditChanged);
	connect(m->miniSearcher, &Gui::MiniSearcher::sigFindNextRow, this, &MiniSearcherViewConnector::selectNext);
	connect(m->miniSearcher, &Gui::MiniSearcher::sigFindPrevRow, this, &MiniSearcherViewConnector::selectPrevious);
}

bool MiniSearcherViewConnector::isActive() const
{
	return (m->miniSearcher && m->miniSearcher->isVisible());
}

void MiniSearcherViewConnector::setExtraTriggers(const QMap<QChar, QString>& map)
{
	m->triggerMap = map;
	if(m->miniSearcher)
	{
		m->miniSearcher->setExtraTriggers(map);
	}
}

bool MiniSearcherViewConnector::handleKeyPress(QKeyEvent* e)
{
	return m->miniSearcher->handleKeyPress(e);
}

void MiniSearcherViewConnector::lineEditChanged(const QString& str)
{
	const auto searchMode = GetSetting(Set::Lib_SearchMode);
	m->currentSearchstring = Library::convertSearchstring(str, searchMode, m->triggerMap.keys());

	const auto resultCount = m->searchView->search(m->currentSearchstring);
	spLog(Log::Info, this) << "found " << resultCount << " search results";
	m->miniSearcher->setNumberResults(resultCount);
}

void MiniSearcherViewConnector::selectNext()
{
	m->searchView->searchNext();
}

void MiniSearcherViewConnector::selectPrevious()
{
	m->searchView->searchPrevious();
}

SearchView::SearchView() = default;

SearchView::~SearchView() noexcept = default;

int SearchView::search(const QString& searchstring)
{
	const auto count = searchModel()->initSearch(searchstring, currentSelectedItem());
	if(count > 0)
	{
		const auto index = searchModel()->searchNext();
		selectSearchResult(index);
	}

	return count;
}

void SearchView::searchNext()
{
	const auto index = searchModel()->searchNext();
	if(index >= 0)
	{
		selectSearchResult(index);
	}
}

void SearchView::searchPrevious()
{
	const auto index = searchModel()->searchPrevious();
	if(index >= 0)
	{
		selectSearchResult(index);
	}
}

struct SearchableTableView::Private
{
	MiniSearcherViewConnector* miniSearcher;

	explicit Private(SearchableTableView* parent) :
		miniSearcher {new MiniSearcherViewConnector(parent, parent)} {}
};

SearchableTableView::SearchableTableView(QWidget* parent) :
	Gui::WidgetTemplate<QTableView> {parent},
	SelectionViewInterface(this),
	m {Pimpl::make<Private>(this)} {}

SearchableTableView::~SearchableTableView() = default;

int SearchableTableView::viewportWidth() const { return viewport()->width(); }

int SearchableTableView::viewportHeight() const { return viewport()->height(); }

QWidget* SearchableTableView::widget() { return this; }

int SearchableTableView::currentSelectedItem() const
{
	const auto index = selectionModel()->currentIndex();
	if(index.isValid())
	{
		return -1;
	}

	return (selectionBehavior() == QAbstractItemView::SelectionBehavior::SelectRows)
	       ? index.row()
	       : index.row() * model()->columnCount() + index.column();
}

void SearchableTableView::selectSearchResult(const int index)
{
	if(selectionBehavior() == QAbstractItemView::SelectionBehavior::SelectRows)
	{
		selectRow(index);
	}

	else
	{
		selectItems({index});
	}
}

void SearchableTableView::keyPressEvent(QKeyEvent* event)
{
	const auto isSelectionEvent = SelectionViewInterface::handleKeyPress(event);
	if(isSelectionEvent)
	{
		return;
	}

	m->miniSearcher->init();
	const auto isMinisearcherEvent = m->miniSearcher->handleKeyPress(event);
	if(isMinisearcherEvent)
	{
		return;
	}

	QTableView::keyPressEvent(event);
}
