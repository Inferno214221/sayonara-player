/* SearchableView.h */

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

#ifndef SEARCHABLEVIEW_H
#define SEARCHABLEVIEW_H

#include "GUI/Utils/Widgets/WidgetTemplate.h"
#include "GUI/Utils/SearchableWidget/SelectionView.h"
#include "GUI/Utils/SearchableWidget/SearchableModel.h"
#include "Utils/Pimpl.h"

#include <QKeyEvent>
#include <QTableView>
#include <QListView>
#include <QTreeView>

class QAbstractItemView;
class QItemSelectionModel;
class ExtraTriggerMap;
class SearchableViewInterface;

class MiniSearcherViewConnector : public QObject
{
	Q_OBJECT
	PIMPL(MiniSearcherViewConnector)

public:
	MiniSearcherViewConnector(SearchableViewInterface* parent);
	~MiniSearcherViewConnector();

	void init();
	bool is_active() const;
	void set_extra_triggers(const QMap<QChar, QString>& map);
	void handle_key_press(QKeyEvent* e);

private slots:
	void edit_changed(const QString& str);
	void select_next();
	void select_previous();
};


/**
 * @brief The SearchViewInterface class
 * @ingroup GUIInterfaces
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
		virtual ~SearchableViewInterface();

		virtual void set_search_model(SearchableModelInterface* model);

		virtual QModelIndex model_index(int row, int col, const QModelIndex& parent=QModelIndex()) const override final;
		virtual int			row_count(const QModelIndex& parent=QModelIndex()) const override final;
		virtual int			column_count(const QModelIndex& parent=QModelIndex()) const override final;
		bool				is_empty(const QModelIndex& parent=QModelIndex()) const;
		bool				has_rows(const QModelIndex& parent=QModelIndex()) const;

		virtual QItemSelectionModel*	selection_model() const override final;
		virtual void					set_current_index(int idx) override final;

		bool			is_minisearcher_active() const;
		virtual int		viewport_height() const;
		virtual int		viewport_width() const;

		QAbstractItemView* view() const;

		int set_searchstring(const QString& str);
		void select_next_match(const QString& str);
		void select_previous_match(const QString& str);

	protected:
		virtual QModelIndex match_index(const QString& str, SearchDirection direction) const;
		virtual void	select_match(const QString& str, SearchDirection direction);
		void			handle_key_press(QKeyEvent* e) override;
};


template<typename View, typename Model>
class SearchableView :
		public View,
		public SearchableViewInterface
{
	private:
		using View::setModel;
		using SearchableViewInterface::set_search_model;

	public:
		SearchableView(QWidget* parent=nullptr) :
			View(parent),
			SearchableViewInterface(this) {}

		virtual ~SearchableView() {}

		virtual void set_model(Model* model)
		{
			setModel(model);
			set_search_model(model);
		}

	protected:
		void keyPressEvent(QKeyEvent* e) override
		{
			if(!e->isAccepted())
			{
				handle_key_press(e);
				if(e->isAccepted()){
					return;
				}
			}

			View::keyPressEvent(e);
		}
};

using SearchableTableView=Gui::WidgetTemplate<SearchableView<QTableView, SearchableTableModel>>;
using SearchableListView=Gui::WidgetTemplate<SearchableView<QListView, SearchableListModel>>;


#endif // SEARCHABLEVIEW_H
