/* SearchableView.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QMap>
#include <QString>
#include <QScrollBar>

struct MiniSearcherViewConnector::Private
{
	QMap<QChar, QString>		trigger_map;
	QString						current_searchstring;

	MiniSearcher*				mini_searcher=nullptr;
	SearchableViewInterface*	svi=nullptr;
};

MiniSearcherViewConnector::MiniSearcherViewConnector(SearchableViewInterface* parent) :
	QObject(parent->view())
{
	m = Pimpl::make<Private>();
	m->svi = parent;
}

MiniSearcherViewConnector::~MiniSearcherViewConnector() {}

void MiniSearcherViewConnector::init()
{
	m->mini_searcher = new MiniSearcher(m->svi);
	m->mini_searcher->set_extra_triggers(m->trigger_map);

	connect(m->mini_searcher, &MiniSearcher::sig_text_changed, this, &MiniSearcherViewConnector::edit_changed);
	connect(m->mini_searcher, &MiniSearcher::sig_find_next_row, this, &MiniSearcherViewConnector::select_next);
	connect(m->mini_searcher, &MiniSearcher::sig_find_prev_row, this, &MiniSearcherViewConnector::select_previous);
}

bool MiniSearcherViewConnector::is_active() const
{
	return (m->mini_searcher && m->mini_searcher->isVisible());
}

void MiniSearcherViewConnector::set_extra_triggers(const QMap<QChar, QString>& map)
{
	m->trigger_map = map;

	if(m->mini_searcher){
		m->mini_searcher->set_extra_triggers(map);
	}
}

void MiniSearcherViewConnector::handle_key_press(QKeyEvent* e)
{
	m->mini_searcher->handle_key_press(e);
}


void MiniSearcherViewConnector::edit_changed(const QString& str)
{
	Library::SearchModeMask search_mode = GetSetting(Set::Lib_SearchMode);
	m->current_searchstring = Library::Utils::convert_search_string(str, search_mode, m->trigger_map.keys());

	int num_results = m->svi->set_searchstring(m->current_searchstring);
	m->mini_searcher->set_number_results(num_results);
}

void MiniSearcherViewConnector::select_next()
{
	m->svi->select_next_match(m->current_searchstring);
}

void MiniSearcherViewConnector::select_previous()
{
	m->svi->select_previous_match(m->current_searchstring);
}



struct SearchableViewInterface::Private :
		public QObject
{
	MiniSearcherViewConnector*	minisearcher=nullptr;
	SearchableModelInterface*	search_model=nullptr;
	SearchableViewInterface*	search_view=nullptr;
	QAbstractItemView*			view=nullptr;

	QModelIndexList				found_search_indexes;
	int							current_search_index;
	int							cur_idx;

	Private(SearchableViewInterface* parent, QAbstractItemView* v) :
		QObject(v),
		search_view(parent),
		view(v),
		cur_idx(-1)
	{}
};


SearchableViewInterface::SearchableViewInterface(QAbstractItemView* view) :
	SelectionViewInterface()
{
	m = Pimpl::make<Private>(this, view);

	m->minisearcher = new MiniSearcherViewConnector(this);
}

SearchableViewInterface::~SearchableViewInterface() {}

bool SearchableViewInterface::is_minisearcher_active() const
{
	if(!m->search_model){
		return false;
	}

	return m->minisearcher->is_active();
}

int SearchableViewInterface::viewport_height() const
{
	return m->view->viewport()->y() + m->view->viewport()->height();
}

int SearchableViewInterface::viewport_width() const
{
	return m->view->viewport()->x() + m->view->viewport()->width();
}

QAbstractItemView* SearchableViewInterface::view() const
{
	return m->view;
}

int SearchableViewInterface::set_searchstring(const QString& str)
{
	m->found_search_indexes = m->search_model->search_results(str);
	m->current_search_index = -1;

	this->select_match(str, SearchDirection::First);

	return m->found_search_indexes.size();
}

void SearchableViewInterface::select_next_match(const QString& str)
{
	this->select_match(str, SearchDirection::Next);
}

void SearchableViewInterface::select_previous_match(const QString& str)
{
	this->select_match(str, SearchDirection::Prev);
}

void SearchableViewInterface::set_search_model(SearchableModelInterface* model)
{
	 m->search_model = model;

	 if(m->search_model)
	 {
		 m->minisearcher->set_extra_triggers(m->search_model->getExtraTriggers());
	 }
}

int SearchableViewInterface::row_count(const QModelIndex& parent) const
{
	return m->view->model()->rowCount(parent);
}

int SearchableViewInterface::column_count(const QModelIndex& parent) const
{
	return m->view->model()->columnCount(parent);
}

bool SearchableViewInterface::is_empty(const QModelIndex& parent) const
{
	return (row_count(parent) == 0);
}

bool SearchableViewInterface::has_rows(const QModelIndex& parent) const
{
	return (row_count(parent) > 0);
}

QModelIndex SearchableViewInterface::model_index(int row, int col, const QModelIndex& parent) const
{
	return m->view->model()->index(row, col, parent);
}


QModelIndex SearchableViewInterface::match_index(const QString& str, SearchDirection direction) const
{
	QModelIndex idx;
	if(str.isEmpty()) {
		return QModelIndex();
	}

	if(!m->search_model) {
		return QModelIndex();
	}

	if(m->found_search_indexes.isEmpty())
	{
		return QModelIndex();
	}

	if(m->current_search_index < 0 || m->current_search_index >= m->found_search_indexes.size()){
		m->current_search_index = 0;
	}

	switch(direction)
	{
		case SearchDirection::First:
			idx = m->found_search_indexes.first();
			m->current_search_index = 0;
			break;

		case SearchDirection::Next:
			m->current_search_index++;
			if(m->current_search_index >= m->found_search_indexes.count()){
				m->current_search_index = 0;
			}

			idx = m->found_search_indexes.at(m->current_search_index);
			break;

		case SearchDirection::Prev:
			m->current_search_index--;
			if(m->current_search_index < 0){
				m->current_search_index = m->found_search_indexes.count() - 1;
			}

			idx = m->found_search_indexes.at(m->current_search_index);
			break;
	}

	return idx;
}


void SearchableViewInterface::select_match(const QString& str, SearchDirection direction)
{
	QModelIndex idx = match_index(str, direction);
	if(!idx.isValid()){
		m->cur_idx = -1;
		return;
	}

	m->cur_idx = index_by_model_index(idx);

	IndexSet indexes(m->cur_idx);

	switch(selection_type())
	{
		case SelectionViewInterface::SelectionType::Rows:
			select_rows(indexes);
			break;
		case SelectionViewInterface::SelectionType::Columns:
			select_columns(indexes);
			break;
		case SelectionViewInterface::SelectionType::Items:
			select_items(indexes);
			break;
	}

	this->set_current_index(m->cur_idx);

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

QItemSelectionModel* SearchableViewInterface::selection_model() const
{
	return m->view->selectionModel();
}

void SearchableViewInterface::set_current_index(int idx)
{
	ModelIndexRange range = model_indexrange_by_index(idx);
	m->view->setCurrentIndex(range.first);
}

void SearchableViewInterface::handle_key_press(QKeyEvent* e)
{
	SelectionViewInterface::handle_key_press(e);

	if(e->isAccepted()) {
		return;
	}

	if(!m->search_model){
		return;
	}

	m->minisearcher->init();
	m->minisearcher->handle_key_press(e);
}
