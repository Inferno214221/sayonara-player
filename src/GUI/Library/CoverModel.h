/* CoverModel.h */

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

#ifndef ALBUMCOVERMODEL_H
#define ALBUMCOVERMODEL_H

#include "GUI/Library/ItemModel.h"
#include "Utils/Pimpl.h"

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
	class CoverModel :
			public ItemModel
	{
		Q_OBJECT
		PIMPL(CoverModel)

		public:
			explicit CoverModel(QObject* parent, AbstractLibrary* library);
			virtual ~CoverModel();

		public:
			int				rowCount(const QModelIndex& parent=QModelIndex()) const override;
			int				columnCount(const QModelIndex& paren=QModelIndex()) const override;
			QVariant		data(const QModelIndex& index, int role) const override;
			Qt::ItemFlags	flags(const QModelIndex &index) const override;

			int				zoom() const;
			QSize			item_size() const;


		protected:
			// ItemModel
			const MetaDataList& mimedata_tracks() const override;
			const SP::Set<Id>&	selections() const override;

			QModelIndexList search_results(const QString& substr) override;

			int				searchable_column() const override;
			QString			searchable_string(int idx) const override;
			int				id_by_index(int idx) const override;
			Cover::Location	cover(const IndexSet& indexes) const override;

		private:
			const AlbumList& albums() const;
			void add_rows(int row, int count);
			void remove_rows(int row, int count);
			void add_columns(int column, int count);
			void remove_columns(int column, int count);
			void refresh_data();

		public slots:
			void set_zoom(int zoom, const QSize& view_size);
			void reload();
			void clear();

		private slots:
			void next_hash();
			void cover_lookup_finished(bool success);
	};
}

#endif // ALBUMCOVERMODEL_H
