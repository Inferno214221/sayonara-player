/* AlternativeCoverItemModel.cpp */

/* Copyright (C) 2011-2017 Lucio Carreras
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


/*
 * AlternativeCoverItemModel.cpp
 *
 *  Created on: Jul 1, 2011
 *      Author: Lucio Carreras
 */

#include "AlternativeCoverItemModel.h"
#include "Components/Covers/CoverLocation.h"
#include "Utils/globals.h"
#include "Utils/Utils.h"

#include <QModelIndex>
#include <QVariant>
#include <QStringList>
#include <QPixmap>
#include <QIcon>

#include <algorithm>

using Cover::Location;

struct AlternativeCoverItemModel::Private
{
	QStringList pathlist;

	Private(int rows, int columns)
	{
		reset_paths(rows, columns);
	}

	void reset_paths(int rows, int columns)
	{
		QString invalid_path = Location::invalid_location().cover_path();

		pathlist.clear();

		for(int i=0; i<rows*columns; i++)
		{
			pathlist << invalid_path;
		}
	}
};

AlternativeCoverItemModel::AlternativeCoverItemModel(QObject* parent) :
	QAbstractTableModel(parent)
{
	m = Pimpl::make<Private>(5, columnCount());
}

AlternativeCoverItemModel::~AlternativeCoverItemModel() {}

RowColumn AlternativeCoverItemModel::cvt_2_row_col(int idx) const
{
	RowColumn p;

	if(idx < 0) {
		p.row = -1;
		p.col = -1;
		p.valid = false;
	}

	p.row = idx / columnCount();
	p.col = idx % columnCount();
	p.valid = true;

	return p;
}


int AlternativeCoverItemModel::cvt_2_idx(int row, int col) const
{
	if(row < 0 || col < 0) return -1;

	return row * columnCount() + col;
}


int AlternativeCoverItemModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return (m->pathlist.size() + columnCount() - 1) /  columnCount();
}


int AlternativeCoverItemModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return 5;
}


QVariant AlternativeCoverItemModel::data(const QModelIndex &index, int role) const
{
	int lin_idx = this->cvt_2_idx(index.row(), index.column());
	if(lin_idx < 0) {
		return QVariant();
	}

	 if ( !index.isValid() || !between(lin_idx, m->pathlist) ) {
		 return QVariant();
	 }

	 else if(role == Qt::UserRole){
		 return m->pathlist[lin_idx];
	 }

	 else if(role == Qt::SizeHintRole){
		const int sz = 80;
		return QSize(sz, sz);
	 }

	 return QVariant();
}


Qt::ItemFlags AlternativeCoverItemModel::flags(const QModelIndex &index) const
{
	if (!index.isValid()){
		return Qt::ItemIsEnabled;
	}

	if(!between(index.row(), m->pathlist)){
		return QAbstractTableModel::flags(index);
	}

	bool invalid = Location::is_invalid(m->pathlist[index.row()]);
	if(invalid){
		return (Qt::NoItemFlags);
	}

	return QAbstractTableModel::flags(index);
}

bool AlternativeCoverItemModel::set_cover(int row, int column, const QString& cover)
{
	int lin_idx = cvt_2_idx(row, column);

	if(lin_idx >= m->pathlist.size() || lin_idx < 0){
		return false;
	}

	m->pathlist[lin_idx] = cover;

	QModelIndex idx = index(row, column);
	emit dataChanged(idx, idx);

	return true;
}


void AlternativeCoverItemModel::reset()
{
	m->reset_paths(rowCount(), columnCount());

	emit dataChanged(index(0, 0), index(rowCount()-1, columnCount() - 1));
}


bool AlternativeCoverItemModel::insertRows(int position, int rows, const QModelIndex &index)
{
	Q_UNUSED(index);

	beginInsertRows(QModelIndex(), position, position+rows-1);

	m->reset_paths(rows, columnCount());

	endInsertRows();
	return true;
}


bool AlternativeCoverItemModel::removeRows(int position, int rows, const QModelIndex &index)
{
	Q_UNUSED(index);

	 beginRemoveRows(QModelIndex(), position, position+rows-1);

	 m->reset_paths(5, columnCount());

	 endRemoveRows();
	 return true;
}


bool AlternativeCoverItemModel::is_valid(int row, int col)
{
	int idx = cvt_2_idx(row, col);
	if(idx < 0) return false;

	return ( !Location::is_invalid(m->pathlist[ idx ]) );
}

QSize AlternativeCoverItemModel::get_cover_size(const QModelIndex& idx) const
{
	QString cover_path = idx.data(Qt::UserRole).toString();
	QPixmap cover(cover_path);
	return cover.size();
}
