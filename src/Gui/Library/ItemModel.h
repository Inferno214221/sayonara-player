/* ItemModel.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

namespace Cover
{
	class Location;
}

class AbstractLibrary;

namespace Library
{
	/**
	 * @brief The ItemModel is intended to abstract the various views. It supports
	 * searching, selections and a library
	 * @ingroup GuiLibrary
	 */
	class ItemModel :
		public SearchableTableModel
	{
		Q_OBJECT
		PIMPL(ItemModel)

		public:
            ItemModel(int columnCount, QObject* parent, AbstractLibrary* library);
			~ItemModel() override;

            QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
            bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value,
                               int role = Qt::EditRole) override;

			int columnCount(const QModelIndex& parent = QModelIndex()) const override;

			QModelIndexList searchResults(const QString& substr) override;

			/**
			 * @brief the index of the searchable column. This is the column
			 * where the text is searched for a certain searchstring
			 */
			virtual int searchableColumn() const = 0;

			/**
			 * @brief here, the searchable string can even be refined. Maybe
			 * we just want to search within a substring indicated by the row
			 * @param row
			 * @return
			 */
			virtual QString searchableString(int row) const = 0;

			/**
			 * @brief return the current id for a given row
			 * @param row
			 * @return
			 */
			virtual Id mapIndexToId(int row) const = 0;

			/**
			 * @brief return the cover for multiple rows. if rows.size() > 1,
			 * an invalid, default constructed cover location is usually shown
			 * @param rows
			 * @return
			 */
			virtual Cover::Location cover(const QModelIndexList& indexList) const = 0;

			/**
			 * @brief return the tracks which belong to the selections. If an
			 * album is selected for example, all tracks of that album should be returned
			 * @return
			 */
			virtual const MetaDataList& selectedMetadata() const = 0;
			virtual QMimeData* mimeData(const QModelIndexList& indexList) const override;

			void refreshData(int* rowCountBefore = nullptr, int* rowCountAfter = nullptr); //returns the size difference

		protected:
			AbstractLibrary* library();
			const AbstractLibrary* library() const;

		private:
			bool removeRows(int position, int rows, const QModelIndex& index = QModelIndex()) override;
			bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
	};
}

#endif /* LIBRARYITEMMODEL_H_ */
