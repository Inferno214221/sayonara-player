/* AlbumModel.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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
 * AlbumModel.h
 *
 *  Created on: Apr 26, 2011
 *      Author: Lucio Carreras
 */

#ifndef LIBRARYITEMMODELALBUMS_H_
#define LIBRARYITEMMODELALBUMS_H_

#include "Gui/Library/ItemModel.h"
#include "Utils/Pimpl.h"

namespace Library
{
	/**
	 * @brief The AlbumModel class
	 * @ingroup GuiLibrary
	 */
	class AlbumModel :
			public ItemModel
	{
		Q_OBJECT
		PIMPL(AlbumModel)

		public:
			AlbumModel(QObject* parent, AbstractLibrary* library);
			~AlbumModel() override;

			Qt::ItemFlags	flags(const QModelIndex &index) const override;
			QVariant		data(const QModelIndex& index, int role) const override;
			bool			setData(const QModelIndex& index, const QVariant& value, int role=Qt::DisplayRole) override;
			int				rowCount(const QModelIndex &parent) const override;

			Cover::Location	cover(const IndexSet& indexes) const override;
			int				searchable_column() const override;
			Id				id_by_index(int index) const override;
			QString			searchable_string(int row) const override;


			const Util::Set<Id>& selections() const override;

		protected:
			const MetaDataList& mimedata_tracks() const override;

		private slots:
			void rating_changed(int row);
	};
}

#endif /* LIBRARYITEMMODELALBUMS_H_ */
