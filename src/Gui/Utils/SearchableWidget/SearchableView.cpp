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
#include "Utils/Settings/Settings.h"
#include "Utils/Set.h"

#include <QListView>
#include <QAbstractItemModel>
#include <QKeyEvent>
#include <QMap>
#include <QString>
#include <QScrollBar>

struct MiniSearcherViewConnector::Private
{
	QMap<QChar, QString>		triggerMap;
	QString						currentSearchstring;

	Gui::MiniSearcher*			miniSearcher=nullptr;
	SearchableViewInterface*	svi=nullptr;
};

MiniSearcherViewConnector::MiniSearcherViewConnector(SearchableViewInterface* parent) :
	QObject(parent->view())
{
	m = Pimpl::make<Private>();
	m->svi = parent;
}

MiniSearcherViewConnector::~MiniSearcherViewConnector() = default;

void MiniSearcherViewConnector::init()
{
	m->miniSearcher = new Gui::MiniSearcher(m->svi);
	m->miniSearcher->set_extra_triggers(m->triggerMap);

	connect(m->miniSearcher, &Gui::MiniSearcher::sig_text_changed, this, &MiniSearcherViewConnector::lineEditChanged);
	connect(m->miniSearcher, &Gui::MiniSearcher::sig_find_next_row, this, &MiniSearcherViewConnector::selectNext);
	connect(m->miniSearcher, &Gui::MiniSearcher::sig_find_prev_row, this, &MiniSearcherViewConnector::selectPrevious);
}

bool MiniSearcherViewConnector::isActive() const
{
	return (m->miniSearcher && m->miniSearcher->isVisible());
}

void MiniSearcherViewConnector::setExtraTriggers(const QMap<QChar, QString>& map)
{
	m->triggerMap = map;

	if(m->miniSearcher){
		m->miniSearcher->set_extra_triggers(map);
	}
}

void MiniSearcherViewConnector::handleKeyPress(QKeyEvent* e)
{
	m->miniSearcher->handle_key_press(e);
}


void MiniSearcherViewConnector::lineEditChanged(const QString& str)
{
	Library::SearchModeMask search_mode = GetSetting(Set::Lib_SearchMode);
	m->currentSearchstring = Library::Utils::convertSearchstring(str, search_mode, m->triggerMap.keys());

	int results = m->svi->setSearchstring(m->currentSearchstring);
	m->miniSearcher->set_number_results(results);
}

void MiniSearcherViewConnector::selectNext()
{
	m->svi->selectNextMatch(m->currentSearchstring);
}

void MiniSearcherViewConnector::selectPrevious()
{
	m->svi->selectPreviousMatch(m->currentSearchstring);
}

struct SearchableViewInterface::Private :
		public QObject
{
	MiniSearcherViewConnector*	minisearcher=nullptr;
	SearchableModelInterface*	searchModel=nullptr;
	SearchableViewInterface*	searchView=nullptr;
	QAbstractItemView*			view=nullptr;

	QModelIndexList				foundSearchIndexes;
	int							currentSearchIndex;
	int							currentIndex;

	Private(SearchableViewInterface* parent, QAbstractItemView* v) :
		QObject(v),
		searchView(parent),
		view(v),
		currentIndex(-1)
	{}
};

SearchableViewInterface::SearchableViewInterface(QAbstractItemView* view) :
	SelectionViewInterface(view)
{
	m = Pimpl::make<Private>(this, view);

	m->minisearcher = new MiniSearcherViewConnector(this);
}

SearchableViewInterface::~SearchableViewInterface() = default;

bool SearchableViewInterface::isMinisearcherActive() const
{
	if(!m->searchModel){
		return false;
	}

	return m->minisearcher->isActive();
}

int SearchableViewInterface::viewportHeight() const
{
	return m->view->viewport()->y() + m->view->viewport()->height();
}

int SearchableViewInterface::viewportWidth() const
{
	return m->view->viewport()->x() + m->view->viewport()->width();
}

QAbstractItemView* SearchableViewInterface::view() const
{
	return m->view;
}

int SearchableViewInterface::setSearchstring(const QString& str)
{
	m->foundSearchIndexes = m->searchModel->searchResults(str);
	m->currentSearchIndex = -1;

	this->selectMatch(str, SearchDirection::First);

	return m->foundSearchIndexes.size();
}

void SearchableViewInterface::selectNextMatch(const QString& str)
{
	this->selectMatch(str, SearchDirection::Next);
}

void SearchableViewInterface::selectPreviousMatch(const QString& str)
{
	this->selectMatch(str, SearchDirection::Prev);
}

void SearchableViewInterface::setSearchModel(SearchableModelInterface* model)
{
	 m->searchModel = model;
	 if(m->searchModel)
	 {
		 m->minisearcher->setExtraTriggers(m->searchModel->getExtraTriggers());
	 }
}


QModelIndex SearchableViewInterface::matchIndex(const QString& str, SearchDirection direction) const
{
	QModelIndex idx;
	if(str.isEmpty()) {
		return QModelIndex();
	}

	if(!m->searchModel) {
		return QModelIndex();
	}

	if(m->foundSearchIndexes.isEmpty())
	{
		return QModelIndex();
	}

	if(m->currentSearchIndex < 0 || m->currentSearchIndex >= m->foundSearchIndexes.size()){
		m->currentSearchIndex = 0;
	}

	switch(direction)
	{
		case SearchDirection::First:
			idx = m->foundSearchIndexes.first();
			m->currentSearchIndex = 0;
			break;

		case SearchDirection::Next:
			m->currentSearchIndex++;
			if(m->currentSearchIndex >= m->foundSearchIndexes.count()){
				m->currentSearchIndex = 0;
			}

			idx = m->foundSearchIndexes.at(m->currentSearchIndex);
			break;

		case SearchDirection::Prev:
			m->currentSearchIndex--;
			if(m->currentSearchIndex < 0){
				m->currentSearchIndex = m->foundSearchIndexes.count() - 1;
			}

			idx = m->foundSearchIndexes.at(m->currentSearchIndex);
			break;
	}

	return idx;
}

void SearchableViewInterface::selectMatch(const QString& str, SearchDirection direction)
{
	QModelIndex idx = matchIndex(str, direction);
	if(!idx.isValid())
	{
		m->currentIndex = -1;
		return;
	}

	m->currentIndex = mapModelIndexToIndex(idx);

	IndexSet indexes(m->currentIndex);

	switch(selectionType())
	{
		case SelectionViewInterface::SelectionType::Rows:
			selectRows(indexes);
			break;
		case SelectionViewInterface::SelectionType::Columns:
			selectColumns(indexes);
			break;
		case SelectionViewInterface::SelectionType::Items:
			selectItems(indexes);
			break;
	}

	m->view->setCurrentIndex(idx);

	if(direction == SearchDirection::First){
		m->view->scrollTo(idx, QListView::ScrollHint::PositionAtCenter);
	}

	else if(direction == SearchDirection::Next){
		m->view->scrollTo(idx, QListView::ScrollHint::PositionAtCenter);
	}

	else if(direction == SearchDirection::Prev){
		m->view->scrollTo(idx, QListView::ScrollHint::PositionAtCenter);
	}
}

void SearchableViewInterface::handleKeyPress(QKeyEvent* e)
{
	SelectionViewInterface::handleKeyPress(e);
	if(e->isAccepted()) {
		return;
	}

	if(!m->searchModel){
		return;
	}

	m->minisearcher->init();
	m->minisearcher->handleKeyPress(e);
}
