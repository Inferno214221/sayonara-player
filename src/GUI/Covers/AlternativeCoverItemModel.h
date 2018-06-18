/* AlternativeCoverItemModel.h */

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
 * AlternativeCoverItemModel.h
 *
 *  Created on: Jul 1, 2011
 *      Author: Lucio Carreras
 */

#ifndef ALTERNATIVECOVERITEMMODEL_H_
#define ALTERNATIVECOVERITEMMODEL_H_

#include "Utils/Pimpl.h"

#include <QModelIndex>
#include <QAbstractTableModel>

struct RowColumn
{
	int row;
	int col;
	bool valid;

	RowColumn()
	{
		row = -1;
		col = -1;
		valid = false;
	}
};


/**
 * @brief The AlternativeCoverItemModel class
 * @ingroup GUICovers
 */
class AlternativeCoverItemModel : public QAbstractTableModel
{
	Q_OBJECT
	PIMPL(AlternativeCoverItemModel)

public:
	explicit AlternativeCoverItemModel(QObject* parent);
	virtual ~AlternativeCoverItemModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const override;

	Qt::ItemFlags flags(const QModelIndex &index) const override;

	bool add_cover(const QString& cover);
	int cover_count() const;
	void reset();

	bool is_valid(int row, int col);
	QSize get_cover_size(const QModelIndex& idx) const;


	RowColumn cvt_2_row_col(int idx) const ;
	int cvt_2_idx(int row, int col) const ;
};

#endif
