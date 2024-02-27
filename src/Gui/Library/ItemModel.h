/* ItemModel.h */

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

#ifndef LIBRARYITEMMODEL_H_
#define LIBRARYITEMMODEL_H_

#include "Gui/Utils/SearchableWidget/SearchableModel.h"
#include "Utils/Pimpl.h"

#include <QModelIndex>

namespace Cover
{
	class Location;
}

class AbstractLibrary;

namespace Library
{
	class ItemModel :
		public SearchableTableModel
	{
		Q_OBJECT
		PIMPL(ItemModel)

		public:
			ItemModel(int columnCount, QObject* parent, AbstractLibrary* library);
			~ItemModel() override;

			[[nodiscard]] QVariant
			headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

			bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value,
			                   int role = Qt::EditRole) override;

			[[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;

			[[nodiscard]] virtual Id mapIndexToId(int row) const = 0;

			[[nodiscard]] virtual Cover::Location cover(const QModelIndexList& indexList) const = 0;

			[[nodiscard]] virtual const MetaDataList& selectedMetadata() const = 0;

			[[nodiscard]] QMimeData* mimeData(const QModelIndexList& indexList) const override;

			virtual void refreshData();

		protected:
			[[nodiscard]] AbstractLibrary* library();
			[[nodiscard]] const AbstractLibrary* library() const;

		private:
			bool removeRows(int position, int rows, const QModelIndex& index = QModelIndex()) override;
			bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
	};
}

#endif /* LIBRARYITEMMODEL_H_ */
