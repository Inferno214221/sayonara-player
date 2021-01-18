/* ArtistModel.h */

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

/*
 * LibraryItemModelArtistts.h
 *
 *  Created on: Apr 26, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef LIBRARYITEMMODELARTISTS_H_
#define LIBRARYITEMMODELARTISTS_H_

#include "Gui/Library/ItemModel.h"

namespace Library
{
	/**
	 * @brief The ArtistModel class
	 * @ingroup GuiLibrary
	 */
	class ArtistModel :
		public ItemModel
	{
		Q_OBJECT

		public:
			ArtistModel(QObject* parent, AbstractLibrary* library);
			~ArtistModel() override;

			/** AbstractSearchTableModel **/
			Qt::ItemFlags flags(const QModelIndex& index) const override;
			QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
			int rowCount(const QModelIndex& parent) const override;

			/** LibraryItemModel **/
			Cover::Location cover(const QModelIndexList& indexes) const override;
			int searchableColumn() const override;
			Id mapIndexToId(int row) const override;
			QString searchableString(int row) const override;

		protected:
			const MetaDataList& selectedMetadata() const override;
	};
}

#endif /* LIBRARYITEMMODELARTISTS_H_ */
