/* SearchableView.h */

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

#ifndef SEARCHABLEVIEW_H
#define SEARCHABLEVIEW_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/SearchableWidget/SelectionView.h"
#include "Utils/Pimpl.h"

#include <QKeyEvent>
#include <QTableView>

class SearchModel;
class MiniSearcherViewConnector;

class SearchView
{
	public:
		SearchView();
		virtual ~SearchView() noexcept;

		SearchView(const SearchView& other) = delete;
		SearchView(SearchView&& other) = delete;
		SearchView& operator=(const SearchView& other) = delete;
		SearchView& operator=(SearchView&& other) = delete;

		int search(const QString& searchstring);
		void searchNext();
		void searchPrevious();

		virtual void selectSearchResult(int index) = 0;

		[[nodiscard]] virtual int viewportWidth() const = 0;

		[[nodiscard]] virtual int viewportHeight() const = 0;

		[[nodiscard]] virtual QWidget* widget() = 0;

	protected:
		[[nodiscard]] virtual SearchModel* searchModel() const = 0;
};

class SearchableTableView :
	public Gui::WidgetTemplate<QTableView>,
	public SearchView,
	public SelectionViewInterface
{
	Q_OBJECT
	PIMPL(SearchableTableView)

	public:
		explicit SearchableTableView(QWidget* parent = nullptr);
		~SearchableTableView() override;

		SearchableTableView(const SearchableTableView& other) = delete;
		SearchableTableView(SearchableTableView&& other) = delete;
		SearchableTableView& operator=(const SearchableTableView& other) = delete;
		SearchableTableView& operator=(SearchableTableView&& other) = delete;

	protected:
		[[nodiscard]] int viewportWidth() const override;
		[[nodiscard]] int viewportHeight() const override;
		[[nodiscard]] QWidget* widget() override;
		[[nodiscard]] int currentSelectedItem() const override;
		void selectSearchResult(int index) override;

		void keyPressEvent(QKeyEvent* event) override;
};

class MiniSearcherViewConnector :
	public QObject
{
	Q_OBJECT
	PIMPL(MiniSearcherViewConnector)

	public:
		MiniSearcherViewConnector(SearchView* searchView, QObject* parent);
		~MiniSearcherViewConnector() override;

		void init();
		[[nodiscard]] bool isActive() const;
		void setExtraTriggers(const QMap<QChar, QString>& map);
		bool handleKeyPress(QKeyEvent* e);

	private slots:
		void lineEditChanged(const QString& str);
		void selectNext();
		void selectPrevious();
};

#endif // SEARCHABLEVIEW_H
