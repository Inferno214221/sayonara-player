/* SearchableView.cpp */

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

#include "SearchableView.h"
#include "SearchableModel.h"
#include "MiniSearcher.h"

#include "Utils/Library/SearchMode.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"

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
	SearchableViewInterface* searchableView;

	explicit Private(SearchableViewInterface* searchableView) :
		searchableView {searchableView} {}
};

MiniSearcherViewConnector::MiniSearcherViewConnector(SearchableViewInterface* searchableView, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(searchableView);
}

MiniSearcherViewConnector::~MiniSearcherViewConnector() = default;

void MiniSearcherViewConnector::init()
{
	m->miniSearcher = new Gui::MiniSearcher(m->searchableView);
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
	m->currentSearchstring = Library::Utils::convertSearchstring(str, searchMode, m->triggerMap.keys());

	const auto resultCount = m->searchableView->setSearchstring(m->currentSearchstring);
	m->miniSearcher->setNumberResults(resultCount);
}

void MiniSearcherViewConnector::selectNext()
{
	m->searchableView->selectNextMatch(m->currentSearchstring);
}

void MiniSearcherViewConnector::selectPrevious()
{
	m->searchableView->selectPreviousMatch(m->currentSearchstring);
}

struct SearchableViewInterface::Private :
	public QObject
{
	SearchableModelInterface* searchModel {nullptr};
	QAbstractItemView* view;
	MiniSearcherViewConnector* miniSearcherViewConnector;

	QModelIndexList foundSearchIndexes;
	int currentSearchIndex {-1};
	int currentIndex {-1};

	explicit Private(SearchableViewInterface* searchableView, QAbstractItemView* view) :
		QObject(view),
		view {view},
		miniSearcherViewConnector {new MiniSearcherViewConnector(searchableView, view)} {}
};

SearchableViewInterface::SearchableViewInterface(QAbstractItemView* view) :
	SelectionViewInterface(view)
{
	m = Pimpl::make<Private>(this, view);
}

SearchableViewInterface::~SearchableViewInterface() = default;

int SearchableViewInterface::viewportHeight() const
{
	return m->view->viewport()->y() + m->view->viewport()->height();
}

int SearchableViewInterface::viewportWidth() const
{
	return m->view->viewport()->x() + m->view->viewport()->width();
}

QAbstractItemView* SearchableViewInterface::view() const { return m->view; }

int SearchableViewInterface::setSearchstring(const QString& str)
{
	m->foundSearchIndexes = m->searchModel->searchResults(str);
	m->currentSearchIndex = -1;

	selectMatch(str, SearchDirection::First);

	return m->foundSearchIndexes.size();
}

void SearchableViewInterface::selectNextMatch(const QString& str)
{
	selectMatch(str, SearchDirection::Next);
}

void SearchableViewInterface::selectPreviousMatch(const QString& str)
{
	selectMatch(str, SearchDirection::Prev);
}

void SearchableViewInterface::setSearchModel(SearchableModelInterface* model)
{
	m->searchModel = model;
	if(m->searchModel)
	{
		m->miniSearcherViewConnector->setExtraTriggers(m->searchModel->getExtraTriggers());
	}
}

QModelIndex SearchableViewInterface::matchIndex(const QString& str, SearchDirection direction) const
{
	if(str.isEmpty() ||
	   !m->searchModel ||
	   m->foundSearchIndexes.isEmpty())
	{
		return {};
	}

	m->currentSearchIndex = std::max(0, m->currentSearchIndex);
	m->currentSearchIndex = std::min(m->foundSearchIndexes.size() - 1, m->currentSearchIndex);

	switch(direction)
	{
		case SearchDirection::First:
			m->currentSearchIndex = 0;
			break;

		case SearchDirection::Next:
			m->currentSearchIndex++;
			if(m->currentSearchIndex >= m->foundSearchIndexes.count())
			{
				m->currentSearchIndex = 0;
			}
			break;

		case SearchDirection::Prev:
			m->currentSearchIndex--;
			if(m->currentSearchIndex < 0)
			{
				m->currentSearchIndex = m->foundSearchIndexes.count() - 1;
			}
			break;
	}

	return m->foundSearchIndexes.at(m->currentSearchIndex);
}

void SearchableViewInterface::selectMatch(const QString& str, SearchDirection direction)
{
	const auto matchedIndex = matchIndex(str, direction);
	if(!matchedIndex.isValid())
	{
		m->currentIndex = -1;
		return;
	}

	m->currentIndex = mapModelIndexToIndex(matchedIndex);

	switch(selectionType())
	{
		case SelectionViewInterface::SelectionType::Rows:
			selectRows({m->currentIndex});
			break;
		case SelectionViewInterface::SelectionType::Items:
			selectItems({m->currentIndex});
			break;
	}

	m->view->setCurrentIndex(matchedIndex);
	m->view->scrollTo(matchedIndex, QListView::ScrollHint::PositionAtCenter);
}

bool SearchableViewInterface::handleKeyPress(QKeyEvent* e)
{
	const auto b = SelectionViewInterface::handleKeyPress(e);
	if(b)
	{
		return true;
	}

	if(!m->searchModel)
	{
		return false;
	}

	m->miniSearcherViewConnector->init();
	return m->miniSearcherViewConnector->handleKeyPress(e);
}

void SearchableViewInterface::searchDone() {}
