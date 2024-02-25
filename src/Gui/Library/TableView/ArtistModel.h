/* ArtistModel.h */

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
	class ArtistModel :
		public ItemModel
	{
		Q_OBJECT

		public:
			ArtistModel(QObject* parent, AbstractLibrary* library);
			~ArtistModel() override;

			[[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
			[[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
			[[nodiscard]] int rowCount(const QModelIndex& parent) const override;

			[[nodiscard]] Cover::Location cover(const QModelIndexList& indexes) const override;
			[[nodiscard]] Id mapIndexToId(int index) const override;

		protected:
			[[nodiscard]] const MetaDataList& selectedMetadata() const override;
			[[nodiscard]] int itemCount() const override;
			[[nodiscard]] QString searchableString(int index, const QString& prefix) const override;
	};
}

#endif /* LIBRARYITEMMODELARTISTS_H_ */
