/* LibraryListModel.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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



#ifndef LIBRARYLISTMODEL_H
#define LIBRARYLISTMODEL_H

#include <QAbstractListModel>
#include "Utils/Pimpl.h"

using LibName=QString;
using LibPath=QString;

class LibraryListModel :
		public QAbstractListModel
{
	Q_OBJECT
	PIMPL(LibraryListModel)

public:
	LibraryListModel(QObject* parent=nullptr);
	~LibraryListModel();

public:
	int rowCount(const QModelIndex& parent=QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role) const override;

	void append_row(const LibName& name, const LibPath& path);
	void rename_row(int row, const LibName& new_name);
	void change_path(int row, const LibPath& path);
	void move_row(int from, int to);
	void remove_row(int row);

	QStringList all_names() const;
	QStringList all_paths() const;
	QString name(int idx) const;
	QString path(int idx) const;

	void reset();
	bool commit();
};

#endif // LIBRARYLISTMODEL_H
