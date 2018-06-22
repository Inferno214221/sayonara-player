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

struct SearchableViewInterface::Private :
		public QObject
{
	Q_OBJECT

public:
	SearchableModelInterface*	search_model=nullptr;
	SearchableViewInterface*	search_view=nullptr;
	QAbstractItemView*			view=nullptr;
	MiniSearcher*				mini_searcher=nullptr;
	int							cur_idx;

private slots:
	void edit_changed(const QString& str);
	void select_next();
	void select_previous();

public:
	Private(SearchableViewInterface* parent, QAbstractItemView* v) :
		QObject(v)
	{
		view = v;
		search_view = parent;
		search_model = nullptr;
		cur_idx = -1;
		mini_searcher = new MiniSearcher(v);

		connect(mini_searcher, &MiniSearcher::sig_text_changed, this, &Private::edit_changed);
		connect(mini_searcher, &MiniSearcher::sig_find_next_row, this, &Private::select_next);
		connect(mini_searcher, &MiniSearcher::sig_find_prev_row, this, &Private::select_previous);
	}
};


SearchableViewInterface::SearchableViewInterface(QAbstractItemView* view) :
	SelectionViewInterface()
{
	m = Pimpl::make<Private>(this, view);
}

SearchableViewInterface::~SearchableViewInterface() {}

bool SearchableViewInterface::is_minisearcher_active() const
{
	if(!m->search_model){
		return false;
	}

	return m->mini_searcher->isVisible();
}

void SearchableViewInterface::set_mini_searcher_padding(int padding)
{
	m->mini_searcher->set_padding(padding);
}


void SearchableViewInterface::set_search_model(SearchableModelInterface* model)
{
	 m->search_model = model;

	 if(m->search_model)
	 {
		 m->mini_searcher->set_extra_triggers(m->search_model->getExtraTriggers());
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


QModelIndex SearchableViewInterface::model_index(int row, int col, const QModelIndex& parent) const
{
	return m->view->model()->index(row, col, parent);
}


QModelIndex SearchableViewInterface::match_index(const QString& str, SearchDirection direction) const
{
	QModelIndex idx;
	if(str.isEmpty()) {
		return idx;
	}

	if(!m->search_model) {
		return idx;
	}

	Library::SearchModeMask search_mode = Settings::instance()->get<Set::Lib_SearchMode>();
	QMap<QChar, QString> extra_triggers = m->search_model->getExtraTriggers();

	QString converted_string = Library::Util::convert_search_string(str, search_mode, extra_triggers.keys());

	switch(direction)
	{
		case SearchDirection::First:
			idx = m->search_model->getFirstRowIndexOf(converted_string);
			break;
		case SearchDirection::Next:
			idx = m->search_model->getNextRowIndexOf(converted_string, m->cur_idx + 1);
			break;
		case SearchDirection::Prev:
			idx = m->search_model->getPrevRowIndexOf(converted_string, m->cur_idx - 1);
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
	QModelIndex index = model_index_by_index(idx);
	m->view->setCurrentIndex(index);
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

	m->mini_searcher->handle_key_press(e);
}


void SearchableViewInterface::Private::edit_changed(const QString& str)
{
	search_view->select_match(str, SearchDirection::First);

	Library::SearchModeMask search_mode = Settings::instance()->get<Set::Lib_SearchMode>();
	QString search_str = Library::Util::convert_search_string(str, search_mode);

	mini_searcher->set_number_results(
		search_model->getNumberResults(search_str)
	);
}

void SearchableViewInterface::Private::select_next()
{
	QString str = mini_searcher->get_current_text();
	search_view->select_match(str, SearchDirection::Next);
}

void SearchableViewInterface::Private::select_previous()
{
	QString str = this->mini_searcher->get_current_text();
	search_view->select_match(str, SearchDirection::Prev);
}


#include "GUI/Utils/GUI/Utils/SearchableWidget/SearchableView.moc"
