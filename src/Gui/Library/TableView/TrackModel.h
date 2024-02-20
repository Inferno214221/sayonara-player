/* TrackModel.h */

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
 * TrackModel.h
 *
 *  Created on: Apr 24, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef LIBRARYITEMMODELTRACKS_H_
#define LIBRARYITEMMODELTRACKS_H_

#include "Gui/Library/ItemModel.h"
#include "Utils/Pimpl.h"

namespace Library
{
	class TrackModel :
		public ItemModel
	{
		Q_OBJECT
		PIMPL(TrackModel)

		public:
			TrackModel(QObject* parent, AbstractLibrary* library);
			~TrackModel() override;

			[[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
			[[nodiscard]] int rowCount(const QModelIndex& parent) const override;
			[[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
			bool setData(const QModelIndex& index, const QVariant& value, int role) override;

			[[nodiscard]] Cover::Location cover(const QModelIndexList& indexes) const override;
			[[nodiscard]] Id mapIndexToId(int row) const override;

		protected:
			[[nodiscard]] const MetaDataList& selectedMetadata() const override;
			[[nodiscard]] int itemCount() const override;
			[[nodiscard]] QString searchableString(int index, const QString& prefix) const override;

			void languageChanged();

		private slots:
			void trackMetaDataChanged(int row);
	};
}

#endif /* LIBRARYITEMMODELTRACKS_H_ */
