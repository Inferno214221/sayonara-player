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
#include "Utils/Logger/Logger.h"

#include <QModelIndex>
#include <QVariant>
#include <QStringList>
#include <QPixmap>
#include <QIcon>

#include <algorithm>
#include <mutex>

using Cover::Location;

struct AlternativeCoverItemModel::Private
{
	QStringList pathlist;

	Private() {}

	void reset()
	{
		pathlist.clear();
	}
};

AlternativeCoverItemModel::AlternativeCoverItemModel(QObject* parent) :
	QAbstractTableModel(parent)
{
	m = Pimpl::make<Private>();
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

	int cols = columnCount();
	return (m->pathlist.size() + (cols - 1)) / cols;
}


int AlternativeCoverItemModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return 5;
}


QVariant AlternativeCoverItemModel::data(const QModelIndex& index, int role) const
{
	int lin_idx = this->cvt_2_idx(index.row(), index.column());
	if(lin_idx < 0) {
		return QVariant();
	}

	 if (!index.isValid()) {
		 return QVariant();
	 }

	 else if(role == Qt::UserRole)
	 {
		 if(between(lin_idx, m->pathlist)){
			return m->pathlist[lin_idx];
		 }
		 else {
			 return Location::invalid_location().cover_path();
		 }
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

bool AlternativeCoverItemModel::add_cover(const QString& cover)
{
	int n_rows = rowCount();
	m->pathlist << cover;

	if(n_rows < rowCount())
	{
		beginInsertRows(QModelIndex(), rowCount(), rowCount() + 1);
		endInsertRows();
	}

	RowColumn rc = cvt_2_row_col(m->pathlist.size() - 1);
	QModelIndex idx = index(rc.row, rc.col);

	emit dataChanged(idx, idx);

	return true;
}

int AlternativeCoverItemModel::cover_count() const
{
	return m->pathlist.size();
}


void AlternativeCoverItemModel::reset()
{
	beginRemoveRows(QModelIndex(), 0, rowCount());
	endRemoveRows();

	m->reset();

	emit dataChanged(index(0, 0), index(rowCount()-1, columnCount() - 1));
}


bool AlternativeCoverItemModel::is_valid(int row, int col)
{
	int idx = cvt_2_idx(row, col);
	if(idx < 0 || !between(idx, m->pathlist)) {
		return false;
	}

	return ( !Location::is_invalid(m->pathlist[ idx ]) );
}

QSize AlternativeCoverItemModel::get_cover_size(const QModelIndex& idx) const
{
	QString cover_path = idx.data(Qt::UserRole).toString();
	QPixmap cover(cover_path);
	return cover.size();
}
