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

#include "Utils/Library/SearchMode.h"

#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QMap>
#include <QString>

class SearchableModelInterface
{
	public:
		using ExtraTriggerMap = QMap<QChar, QString>;

		virtual ExtraTriggerMap getExtraTriggers();
		virtual QModelIndexList searchResults(const QString& substr) = 0;

		virtual ::Library::SearchModeMask searchMode() const final;

	protected:
		SearchableModelInterface();
		virtual ~SearchableModelInterface();
};

template<typename Model>
class SearchableModel :
	public SearchableModelInterface,
	public Model
{
	public:
		SearchableModel(QObject* parent = nullptr) :
			SearchableModelInterface(),
			Model(parent) {}

		virtual ~SearchableModel() = default;

		using Model::rowCount;
};

using SearchableTableModel = SearchableModel<QAbstractTableModel>;

#endif // GUI_SEARCHABLE_MODEL_H
