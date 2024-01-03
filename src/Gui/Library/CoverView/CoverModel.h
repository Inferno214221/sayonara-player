/* CoverModel.h */

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

#ifndef ALBUMCOVERMODEL_H
#define ALBUMCOVERMODEL_H

#include "Gui/Library/ItemModel.h"
#include "Utils/Pimpl.h"
#include <QtGlobal>

class Album;
class AlbumList;

class QSize;

namespace Cover
{
	class Location;
	class Lookup;
}

namespace Library
{
	/**
	 * @brief The CoverModel class
	 * @ingroup GuiLibrary
	 */
	class CoverModel :
		public ItemModel
	{
		Q_OBJECT
		PIMPL(CoverModel)

		public:
			enum Role
			{
				AlbumRole = Qt::UserRole,
				ArtistRole = Qt::UserRole + 1,
				CoverRole = Qt::UserRole + 2
			};

			explicit CoverModel(QObject* parent, AbstractLibrary* library);
			~CoverModel() override;

		public:
			int rowCount(const QModelIndex& parent = QModelIndex()) const override;
			int columnCount(const QModelIndex& paren = QModelIndex()) const override;
			QVariant data(const QModelIndex& index, int role) const override;
			Qt::ItemFlags flags(const QModelIndex& index) const override;

			QSize itemSize() const;
			int zoom() const;

		protected:
			QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
			const MetaDataList& selectedMetadata() const override;

			QModelIndexList searchResults(const QString& substr) override;

			int searchableColumn() const override;
			QString searchableString(int index) const override;
			int mapIndexToId(int index) const override;
			Cover::Location cover(const QModelIndexList& indexes) const override;

		private:
			const AlbumList& albums() const;
			bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
			bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
			bool insertColumns(int column, int count, const QModelIndex& parent = QModelIndex()) override;
			bool removeColumns(int column, int count, const QModelIndex& parent = QModelIndex()) override;
			void refreshData();

		public slots:
			void setZoom(int zoom, const QSize& view_size);
			void reload();
			void clear();

		private slots:
			void nextHash();
			void coverLookupFinished(bool success);
			void showArtistsChanged();
	};
}

#endif // ALBUMCOVERMODEL_H
