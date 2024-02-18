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

namespace
{
	using Gui::MiniSearcher;
	class QObjectWrapper :
		public QObject
	{
		Q_OBJECT

		public:
			explicit QObjectWrapper(SearchView* view) :
				QObject {nullptr},
				m_view {view} {}

			~QObjectWrapper() override = default;

			void init()
			{
				m_miniSearcher = new MiniSearcher(m_view);
				m_miniSearcher->setSearchOptions(m_view->searchOptions());

				connect(m_miniSearcher, &MiniSearcher::sigTextChanged, this, &QObjectWrapper::searchTextChanged);
				connect(m_miniSearcher, &MiniSearcher::sigFindNextRow, this, [this]() { m_view->searchNext(); });
				connect(m_miniSearcher, &MiniSearcher::sigFindPrevRow, this, [this]() { m_view->searchPrevious(); });
			}

			bool handleKeyPress(QKeyEvent* event)
			{
				if(!m_miniSearcher)
				{
					init();
				}
				return m_miniSearcher->handleKeyPress(event);
			}

		private slots:

			void searchTextChanged(const QString& text)
			{
				const auto resultCount = m_view->search(text);
				m_miniSearcher->setNumberResults(resultCount);
			};

		private:
			SearchView* m_view;
			Gui::MiniSearcher* m_miniSearcher {nullptr};
	};
}

struct SearchView::Private
{
	QObjectWrapper qObjectWrapper;

	explicit Private(SearchView* view) :
		qObjectWrapper {view} {}
};

SearchView::SearchView() :
	m {Pimpl::make<Private>(this)} {}

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

bool SearchView::handleKeyPress(QKeyEvent* event) { return m->qObjectWrapper.handleKeyPress(event); }

QMap<QString, QString> SearchView::searchOptions() const { return searchModel()->searchOptions(); }

SearchableTableView::SearchableTableView(QWidget* parent) :
	Gui::WidgetTemplate<QTableView> {parent},
	SelectionViewInterface(this) {}

SearchableTableView::~SearchableTableView() = default;

QRect SearchableTableView::viewportGeometry() const { return QTableView::geometry(); }

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
	if(const auto isSelectionEvent = SelectionViewInterface::handleKeyPress(event); isSelectionEvent)
	{
		return;
	}

	if(const auto isMinisearcherEvent = SearchView::handleKeyPress(event); isMinisearcherEvent)
	{
		return;
	}

	QTableView::keyPressEvent(event);
}

#include "SearchableView.moc"
