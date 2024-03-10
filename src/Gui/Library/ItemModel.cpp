/* LibraryItemModel.cpp */

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

#include "ItemModel.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"
#include "Gui/Library/Header/ColumnHeader.h"
#include "Gui/Utils/MimeData/CustomMimeData.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <QUrl>

using namespace Library;

namespace
{
	QStringList createEmptyHeader(const int size)
	{
		auto result = QStringList {};
		result.reserve(size);
		std::generate_n(std::back_inserter(result), size, []() -> QString { return {}; });
		return result;
	}
}

struct ItemModel::Private
{
	AbstractLibrary* library;
	QStringList headerNames;
	int oldRowCount {0};

	Private(const int columnCount, AbstractLibrary* library) :
		library {library},
		headerNames {createEmptyHeader(columnCount)} {}
};

ItemModel::ItemModel(const int columnCount, QObject* parent, AbstractLibrary* library) :
	SearchableTableModel(parent),
	m {Pimpl::make<ItemModel::Private>(columnCount, library)} {}

ItemModel::~ItemModel() = default;

QVariant ItemModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
	return ((role == Qt::DisplayRole) && Util::between(section, m->headerNames))
	       ? m->headerNames[section]
	       : SearchableTableModel::headerData(section, orientation, role);
}

bool ItemModel::setHeaderData(const int section, Qt::Orientation orientation, const QVariant& value, const int role)
{
	if(role == Qt::DisplayRole && Util::between(section, m->headerNames))
	{
		m->headerNames[section] = value.toString();
		emit headerDataChanged(orientation, section, section);
		return true;
	}

	return QAbstractItemModel::setHeaderData(section, orientation, value, role);
}

int ItemModel::columnCount(const QModelIndex& /*parent*/) const { return m->headerNames.size(); }

bool ItemModel::removeRows(const int row, const int count, const QModelIndex& /*index*/)
{
	beginRemoveRows(QModelIndex(), row, row + count - 1);
	m->oldRowCount -= count;
	endRemoveRows();

	return true;
}

bool ItemModel::insertRows(const int row, const int count, const QModelIndex& /*index*/)
{
	beginInsertRows(QModelIndex(), row, row + count - 1);
	m->oldRowCount += count;
	endInsertRows();

	return true;
}

void ItemModel::refreshData()
{
	const auto oldSize = m->oldRowCount;
	const auto newSize = rowCount();

	if(oldSize > newSize)
	{
		removeRows(newSize, oldSize - newSize);
	}

	else if(oldSize < newSize)
	{
		insertRows(oldSize, newSize - oldSize);
	}

	// NOLINTNEXTLINE(readability-misleading-indentation)
	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

QMimeData* ItemModel::mimeData(const QModelIndexList& indexes) const
{
	auto* mimeData = new Gui::CustomMimeData(this);

	const auto tracks = selectedMetadata();
	mimeData->setMetadata(tracks);

	const auto coverLocation = this->cover(indexes);
	mimeData->setCoverUrl(coverLocation.preferredPath());

	return mimeData;
}

QString ItemModel::mergeSuggestion(const int itemIndex) const
{
	return searchableString(itemIndex, {});
}

AbstractLibrary* ItemModel::library() { return m->library; }

const AbstractLibrary* ItemModel::library() const { return m->library; }
