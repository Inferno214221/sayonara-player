/* LibraryItemModel.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "ItemModel.h"
#include "Components/Library/AbstractLibrary.h"
#include "GUI/Library/Utils/ColumnHeader.h"
#include "GUI/Utils/CustomMimeData.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Utils/globals.h"
#include "Utils/Set.h"
#include "Utils/Logger/Logger.h"

#include <algorithm>
#include <QUrl>

using namespace Library;

struct ItemModel::Private
{
	AbstractLibrary*	library=nullptr;
	QStringList			header_names;
	int                 old_row_count;


	Private(AbstractLibrary* library) :
		library(library),
		old_row_count(0)
	{}
};


ItemModel::ItemModel(QObject* parent, AbstractLibrary* library) :
	SearchableTableModel(parent)
{
	m = Pimpl::make<ItemModel::Private>(library);
}

ItemModel::~ItemModel() {}

QVariant ItemModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
	if (role != Qt::DisplayRole){
		return QVariant();
	}

	if(!between(section, m->header_names)){
		return QVariant();
	}

	if (orientation == Qt::Horizontal){
		return m->header_names[section];
	}

	return QVariant();
}

bool ItemModel::set_header_data(const QStringList& names)
{
   m->header_names = names;
   emit headerDataChanged(Qt::Horizontal, 0, names.size());
   return true;
}


int ItemModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);

	return m->header_names.size();
}

bool ItemModel::removeRows(int row, int count, const QModelIndex& index)
{
	Q_UNUSED(index)

	beginRemoveRows(QModelIndex(), row, row + count - 1);
	m->old_row_count -= count;
	endRemoveRows();

	return true;
}

bool ItemModel::insertRows(int row, int count, const QModelIndex& index)
{
	Q_UNUSED(index)

	beginInsertRows(QModelIndex(), row, row + count - 1);
	m->old_row_count += count;
	endInsertRows();

	return true;
}


void ItemModel::refresh_data(int* n_rows_before, int* n_rows_after)
{
	int old_size = m->old_row_count;
	int new_size = rowCount();

	if(n_rows_before != nullptr){
		*n_rows_before = old_size;
	}

	if(n_rows_after != nullptr){
		*n_rows_after = new_size;
	}

	if(old_size > new_size){
		removeRows(new_size, old_size - new_size);
	}

	else if(old_size < new_size){
		insertRows(old_size, new_size - old_size);
	}

	emit dataChanged(index(0,0), index(rowCount(), columnCount()));
}


CustomMimeData* ItemModel::custom_mimedata() const
{
	CustomMimeData* mimedata = new CustomMimeData(this);
	MetaDataList v_md = mimedata_tracks();
	mimedata->set_metadata(v_md);

	return mimedata;
}

bool ItemModel::is_selected(int id) const
{
	return selections().contains(id);
}

IndexSet ItemModel::selected_indexes() const
{
	IndexSet ret;

	for(int i=0; i<rowCount(); i++)
	{
		Id id = id_by_index(i);
		if(id < 0){
			continue;
		}

		if(is_selected(id)){
			ret << i;
		}
	}

	return ret;
}

QModelIndex ItemModel::getNextRowIndexOf(const QString& substr, int row, const QModelIndex& parent)
{
	Q_UNUSED(parent)

	int len = rowCount();
	if(len == 0) {
		return QModelIndex();
	}

	for(int i=0; i<len; i++)
	{
		int row_idx = (i + row) % len;

		QString title = searchable_string(row_idx);
		title = Library::Util::convert_search_string(title, search_mode());

		if(title.contains(substr)) {
			return this->index(row_idx, searchable_column());
		}
	}

	return QModelIndex();
}


QModelIndex ItemModel::getPrevRowIndexOf(const QString& substr, int row, const QModelIndex& parent)
{
	Q_UNUSED(parent)

	int len = rowCount();
	if(len < row) row = len - 1;

	for(int i=0; i<len; i++)
	{
		if(row - i < 0) {
			row = len - 1;
		}

		int row_idx = (row - i) % len;

		QString title = searchable_string(row_idx);
		title = Library::Util::convert_search_string(title, search_mode());

		if(title.contains(substr)) {
			return this->index(row_idx, searchable_column());
		}
	}

	return QModelIndex();
}


AbstractLibrary* ItemModel::library()
{
	return m->library;
}

const AbstractLibrary* ItemModel::library() const
{
	return m->library;
}
