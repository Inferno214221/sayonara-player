/* PlaylistItemModel.h */

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
 * PlaylistItemModel.h
 *
 *  Created on: Apr 8, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef PLAYLISTITEMMODEL_H_
#define PLAYLISTITEMMODEL_H_

#include "Gui/Utils/SearchableWidget/SearchableModel.h"
#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Pimpl.h"

class CustomMimeData;

namespace Playlist
{
	/**
	 * @brief The PlaylistItemModel class
	 * @ingroup GuiPlaylists
	 */
	class Model :
			public SearchableTableModel
	{
		Q_OBJECT
		PIMPL(Model)

		using SearchableModelInterface::ExtraTriggerMap;

		signals:
			void sigDataReady();

		public:
			enum StyleElement
			{
				Italic=0x2110,
				Bold=0x212C
			};

			enum ColumnName
			{
				TrackNumber=0,
				Cover,
				Description,
				Time,
				NumColumns
			};

			enum Roles
			{
				RatingRole=Qt::UserRole + 1,
				RadioModeRole=Qt::UserRole + 2,
				DragIndexRole=Qt::UserRole + 3,
				EntryLookRole=Qt::UserRole + 4
			};

			explicit Model(PlaylistPtr pl, QObject* parent=nullptr);
			~Model() override;

			void clear();
			void removeTracks(const IndexSet& rows);
			IndexSet moveTracks(const IndexSet& rows, int target_index);
			IndexSet moveTracksUp(const IndexSet& rows);
			IndexSet moveTracksDown(const IndexSet& rows);
			IndexSet copyTracks(const IndexSet& rows, int target_index);
			void insertTracks(const MetaDataList& tracks, int row);
			void insertTracks(const QStringList& files, int row);

			int	currentTrack() const;
			void setCurrentTrack(int row);

			MetaData metadata(int row) const;
			MetaDataList metadata(const IndexSet& rows) const;

			bool hasLocalMedia(const IndexSet& rows) const;
			void setDragIndex(int dragIndex);
			void changeRating(const IndexSet& rows, Rating rating);

			QModelIndex	getRowIndexOf(const QString& substr, int row, bool is_forward);

			Qt::ItemFlags flags(const QModelIndex& index=QModelIndex()) const override;
			QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;
			bool setData(const QModelIndex& index, const QVariant &value, int role) override;
			int rowCount(const QModelIndex& parent=QModelIndex()) const override;
			int columnCount(const QModelIndex& parent=QModelIndex()) const override;

			SearchableModelInterface::ExtraTriggerMap getExtraTriggers() override;
			QMimeData* mimeData(const QModelIndexList& indexes) const override;
			QModelIndexList	searchResults(const QString& substr) override;

		public slots:
			void refreshData();

		private slots:
			void playlistChanged(int playlistIndex);

		private:
			void lookChanged();
	};
}

#endif /* PLAYLISTITEMMODEL_H_ */
