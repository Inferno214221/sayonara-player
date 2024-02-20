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
#include <QMap>
#include <QTableView>

class SearchModel;

class SearchView
{
	PIMPL(SearchView)

	public:
		SearchView();
		virtual ~SearchView() noexcept;

		SearchView(const SearchView& other) = delete;
		SearchView(SearchView&& other) = delete;
		SearchView& operator=(const SearchView& other) = delete;
		SearchView& operator=(SearchView&& other) = delete;

		[[nodiscard]] virtual QRect viewportGeometry() const = 0;
		int search(const QString& searchstring);
		void searchNext();
		void searchPrevious();

		[[nodiscard]] virtual QWidget* widget() = 0;
		[[nodiscard]] virtual QMap<QString, QString> searchOptions() const;
		[[nodiscard]] virtual QMap<QString, QString> commands() const;

		virtual void triggerResult();
		virtual void runCommand(const QString& command);

	protected:
		[[nodiscard]] virtual SearchModel* searchModel() const = 0;
		virtual void selectSearchResult(int index) = 0;
		[[nodiscard]] virtual int currentSelectedItem() const = 0;
		bool handleKeyPress(QKeyEvent* event);
};

class SearchableTableView :
	public Gui::WidgetTemplate<QTableView>,
	public SearchView,
	public SelectionViewInterface
{
	Q_OBJECT

	public:
		explicit SearchableTableView(QWidget* parent = nullptr);
		~SearchableTableView() override;

		SearchableTableView(const SearchableTableView& other) = delete;
		SearchableTableView(SearchableTableView&& other) = delete;
		SearchableTableView& operator=(const SearchableTableView& other) = delete;
		SearchableTableView& operator=(SearchableTableView&& other) = delete;

	protected:
		[[nodiscard]] QRect viewportGeometry() const override;
		[[nodiscard]] QWidget* widget() override;
		[[nodiscard]] int currentSelectedItem() const override;
		void selectSearchResult(int index) override;

		void keyPressEvent(QKeyEvent* event) override;
};

#endif // SEARCHABLEVIEW_H
