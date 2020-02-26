/* SearchableView.h */

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

#ifndef SEARCHABLEVIEW_H
#define SEARCHABLEVIEW_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/SearchableWidget/SelectionView.h"
#include "Gui/Utils/SearchableWidget/SearchableModel.h"
#include "Utils/Pimpl.h"

#include <QKeyEvent>
#include <QTableView>
#include <QListView>
#include <QTreeView>

class QAbstractItemView;
class QItemSelectionModel;
class ExtraTriggerMap;
class SearchableViewInterface;

/**
 * @brief The MiniSearcherViewConnector class
 * @ingrou Searchable
 */
class MiniSearcherViewConnector : public QObject
{
	Q_OBJECT
	PIMPL(MiniSearcherViewConnector)

	public:
		MiniSearcherViewConnector(SearchableViewInterface* parent);
		~MiniSearcherViewConnector();

		void init();
		bool isActive() const;
		void setExtraTriggers(const QMap<QChar, QString>& map);
		bool handleKeyPress(QKeyEvent* e);

	private slots:
		void lineEditChanged(const QString& str);
		void selectNext();
		void selectPrevious();
};


/**
 * @brief The SearchViewInterface class
 * @ingroup Searchable
 */
class SearchableViewInterface :
		public SelectionViewInterface
{
	PIMPL(SearchableViewInterface)

protected:
	enum class SearchDirection : unsigned char
	{
		First,
		Next,
		Prev
	};

	public:
		explicit SearchableViewInterface(QAbstractItemView* view);
		virtual ~SearchableViewInterface() override;

		QAbstractItemView* view() const;
		virtual int	viewportHeight() const;
		virtual int	viewportWidth() const;

		int setSearchstring(const QString& str);
		void selectNextMatch(const QString& str);
		void selectPreviousMatch(const QString& str);
		bool isMinisearcherActive() const;

	protected:
		virtual void setSearchModel(SearchableModelInterface* model);
		virtual QModelIndex matchIndex(const QString& str, SearchDirection direction) const;
		virtual void selectMatch(const QString& str, SearchDirection direction);
		bool handleKeyPress(QKeyEvent* e) override;
};


template<typename View, typename Model>
class SearchableView :
		public View,
		public SearchableViewInterface
{
	private:
		using View::setModel;
		using SearchableViewInterface::setSearchModel;

	public:
		SearchableView(QWidget* parent=nullptr) :
			View(parent),
			SearchableViewInterface(this)
		{}

		virtual ~SearchableView() = default;

		virtual void setSearchableModel(Model* model)
		{
			View::setModel(model);
			SearchableViewInterface::setSearchModel(model);
		}

		int rowCount() const
		{
			return (View::model() == nullptr) ? 0 : View::model()->rowCount();
		}

	protected:
		void keyPressEvent(QKeyEvent* e) override
		{
			bool processed = handleKeyPress(e);
			if(processed){
					return;
			}

			View::keyPressEvent(e);
		}
};

using SearchableTableView=Gui::WidgetTemplate<SearchableView<QTableView, SearchableTableModel>>;
using SearchableListView=Gui::WidgetTemplate<SearchableView<QListView, SearchableListModel>>;


#endif // SEARCHABLEVIEW_H
