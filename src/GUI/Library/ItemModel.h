/* ItemModel.h */

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

#ifndef LIBRARYITEMMODEL_H_
#define LIBRARYITEMMODEL_H_

#include "GUI/Utils/SearchableWidget/SearchableModel.h"

#include "Utils/Pimpl.h"

namespace Cover
{
	class Location;
}

class CustomMimeData;
class MetaDataList;
class AbstractLibrary;

namespace Library
{
	class ItemModel :
			public SearchableTableModel
	{
		Q_OBJECT
		PIMPL(ItemModel)

		public:
			ItemModel(QObject* parent, AbstractLibrary* library);
			virtual ~ItemModel();

			/** Overloaded from QAbstractTableModel **/
			QVariant		headerData ( int section, Qt::Orientation orientation, int role=Qt::DisplayRole ) const override;
			bool			set_header_data(const QStringList& names);

			virtual int     columnCount(const QModelIndex& parent=QModelIndex()) const override;

			void 			refresh_data(int* n_rows_before=nullptr, int* n_rows_after=nullptr); //returns the size difference

			/** AbstractSearchTableModel **/
			virtual QModelIndex		getNextRowIndexOf(const QString& substr, int row, const QModelIndex& parent=QModelIndex()) override;
			virtual QModelIndex		getPrevRowIndexOf(const QString& substr, int row, const QModelIndex& parent=QModelIndex()) override;

			virtual bool			is_selected(int id) const final;
			virtual const IndexSet& selections() const=0;

			virtual int				searchable_column() const=0;
			virtual QString			searchable_string(int row) const=0;
			virtual Id				id_by_row(int row)=0;
			virtual Cover::Location	cover(const IndexSet& indexes) const=0;

			virtual const MetaDataList&	mimedata_tracks() const=0;
			CustomMimeData*				custom_mimedata() const;

		protected:
			AbstractLibrary*		library();
			const AbstractLibrary*	library() const;

		private:
			bool removeRows(int position, int rows, const QModelIndex& index=QModelIndex()) override;
			bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex()) override;

	};
}

#endif /* LIBRARYITEMMODEL_H_ */
