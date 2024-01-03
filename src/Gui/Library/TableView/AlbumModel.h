/* AlbumModel.h */

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
 * AlbumModel.h
 *
 *  Created on: Apr 26, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef LIBRARYITEMMODELALBUMS_H_
#define LIBRARYITEMMODELALBUMS_H_

#include "Gui/Library/ItemModel.h"
#include "Utils/Pimpl.h"

namespace Tagging
{
	class TagReader;
	class TagWriter;
}

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
			AlbumModel(const std::shared_ptr<Tagging::TagReader>& tagReader,
			           const std::shared_ptr<Tagging::TagWriter>& tagWriter,
			           AbstractLibrary* library, QObject* parent);
			~AlbumModel() override;

			Qt::ItemFlags flags(const QModelIndex& index) const override;
			QVariant data(const QModelIndex& index, int role) const override;
			bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;
			int rowCount(const QModelIndex& parent) const override;

			Cover::Location cover(const QModelIndexList& indexes) const override;
			int searchableColumn() const override;
			Id mapIndexToId(int index) const override;
			QString searchableString(int row) const override;

		protected:
			const MetaDataList& selectedMetadata() const override;

		private slots:
			void albumChanged(int index);
	};
}

#endif /* LIBRARYITEMMODELALBUMS_H_ */
