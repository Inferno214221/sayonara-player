/* AbstractSearchModel.h */

/* Copyright (C) 2011-2020 Lucio Carreras
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

#ifndef ABSTRACT_SEARCH_MODEL_H_
#define ABSTRACT_SEARCH_MODEL_H_

#include <QAbstractTableModel>
#include <QAbstractListModel>
#include <QMap>
#include <QString>
#include "Utils/Library/SearchMode.h"

// We need this for eventual disambiguation between the
// table itself and this interface
// in the Searchable View class

/**
 * @brief The SearchableModelInterface class
 * @ingroup Searchable
 */
class SearchableModelInterface
{
public:
	using ExtraTriggerMap=QMap<QChar, QString>;

	virtual ExtraTriggerMap getExtraTriggers();
	virtual QModelIndexList search_results(const QString& substr)=0;
	virtual bool has_items() const=0;

	virtual ::Library::SearchModeMask search_mode() const final;

protected:
	SearchableModelInterface();
	virtual ~SearchableModelInterface();
};


template <typename Model>
class SearchableModel :
	public SearchableModelInterface,
	public Model
{
	public:
		SearchableModel(QObject* parent=nullptr) :
			SearchableModelInterface(),
			Model(parent)
		{}

		virtual ~SearchableModel() = default;

		using Model::rowCount;

		virtual bool has_items() const override
		{
			return (rowCount() > 0);
		}
};


using SearchableTableModel=SearchableModel<QAbstractTableModel>;
using SearchableListModel=SearchableModel<QAbstractListModel> ;

#endif
