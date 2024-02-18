/* AbstractSearchModel.h */

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

#ifndef GUI_SEARCHABLE_MODEL_H
#define GUI_SEARCHABLE_MODEL_H

#include "Utils/Pimpl.h"
#include "Utils/Library/SearchMode.h"

#include <QAbstractTableModel>
#include <QList>

class QString;
class SearchModel
{
	PIMPL(SearchModel)

	public:
		SearchModel();
		virtual ~SearchModel() noexcept;

		SearchModel(const SearchModel& other) = delete;
		SearchModel(SearchModel&& other) noexcept = delete;
		SearchModel& operator=(const SearchModel& other) = delete;
		SearchModel& operator=(SearchModel&& other) = delete;

		int searchPrevious();
		int searchNext();

		[[nodiscard]] int initSearch(const QString& searchstring, int offsetIndex);
		[[nodiscard]] virtual QString searchableString(int index, const QString& prefix) const = 0;
		[[nodiscard]] virtual int itemCount() const = 0;
};

class SearchableTableModel :
	public QAbstractTableModel,
	public SearchModel
{
	public:
		explicit SearchableTableModel(QObject* parent = nullptr);
		~SearchableTableModel() override;
};

#endif // GUI_SEARCHABLE_MODEL_H
