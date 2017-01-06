/* AbstractSearchModel.h */

/* Copyright 2011-2017  Lucio Carreras
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

#include <QModelIndex>
#include <QAbstractTableModel>
#include <QAbstractListModel>
#include <QMap>
#include <QString>
#include "Helper/Library/SearchMode.h"

// We need this for eventual disambiguation between the
// table itself and this interface
// in the Searchable View class
class SearchModelInterface
{

public:
	virtual QModelIndex getFirstRowIndexOf(const QString& substr)=0;
	virtual QModelIndex getNextRowIndexOf(const QString& substr, int cur_row, const QModelIndex& parent=QModelIndex())=0;
	virtual QModelIndex getPrevRowIndexOf(const QString& substr, int cur_row, const QModelIndex& parent=QModelIndex())=0;
    virtual QMap<QChar, QString> getExtraTriggers()=0;
	
	virtual void set_search_mode(Library::SearchModeMask search_mode);
	virtual Library::SearchModeMask search_mode() const final;

protected:
	SearchModelInterface();
	virtual ~SearchModelInterface();

private:
	Library::SearchModeMask _search_mode;
};


// Searchable Model for tables
class AbstractSearchTableModel :
	public QAbstractTableModel,
	public SearchModelInterface
{

protected:
	explicit AbstractSearchTableModel(QObject* parent=nullptr);
    virtual ~AbstractSearchTableModel();
};


// Searchable Model for lists
class AbstractSearchListModel :
		public QAbstractListModel,
		public SearchModelInterface
{

protected:
	 explicit AbstractSearchListModel(QObject* parent=nullptr);
     virtual ~AbstractSearchListModel();
};


#endif
