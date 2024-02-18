/* PlaylistItemModel.h */

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
 * PlaylistItemModel.h
 *
 *  Created on: Apr 8, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef PLAYLISTITEMMODEL_H
#define PLAYLISTITEMMODEL_H

#include "Gui/Utils/SearchableWidget/SearchableModel.h"

#include "Utils/Library/Sortorder.h"
#include "Utils/Pimpl.h"
#include "Utils/Playlist/PlaylistFwd.h"

#include <QTableView>

namespace Library
{
	class InfoAccessor;
}
class CustomMimeData;
class QPixmap;

namespace Playlist
{
	class Model :
		public SearchableTableModel
	{
		Q_OBJECT
		PIMPL(Model)

		signals:
			void sigDataReady();
			void sigCurrentTrackChanged(int index);
			void sigCurrentScannedFileChanged(const QString& filename);
			void sigBusyChanged(bool b);

		public:
			enum StyleElement
			{
				Italic = 0x2110,
				Bold = 0x212C
			};

			enum ColumnName
			{
				TrackNumber = 0,
				Cover,
				Description,
				Time,
				NumColumns
			};

			enum Roles
			{
				RatingRole = Qt::UserRole + 1,
				DragIndexRole = Qt::UserRole + 2,
				EntryLookRole = Qt::UserRole + 3,
				CurrentPlayingRole = Qt::UserRole + 4,
				EnabledRole = Qt::UserRole + 5
			};

			Model(const PlaylistPtr& playlist, Library::InfoAccessor* libraryInfoAccessor, QObject* parent);
			~Model() override;

			[[nodiscard]] int playlistIndex() const;

			void clear();
			void removeTracks(const IndexSet& rows);
			void deleteTracks(const IndexSet& rows);

			void findTrack(int index);

			IndexSet moveTracks(const IndexSet& rows, int targetIndex);
			IndexSet moveTracksUp(const IndexSet& rows);
			IndexSet moveTracksDown(const IndexSet& rows);
			IndexSet copyTracks(const IndexSet& rows, int targetIndex);
			void insertTracks(const MetaDataList& tracks, int row);
			void insertTracks(const QStringList& files, int row);

			[[nodiscard]] int currentTrack() const;

			[[nodiscard]] MetaDataList metadata(const IndexSet& rows) const;
			[[nodiscard]] bool isEnabled(const int row) const;

			[[nodiscard]] bool hasLocalMedia(const IndexSet& rows) const;
			[[nodiscard]] bool isLocked() const;
			void setLocked(bool b);
			void setDragIndex(int dragIndex);
			void changeRating(const IndexSet& rows, Rating rating);
			void changeTrack(int trackIndex, Seconds seconds = 0);

			void setBusy(bool b);

			[[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override;
			[[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
			bool setData(const QModelIndex& index, const QVariant& value, int role) override;
			[[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
			[[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;

			[[nodiscard]] QMimeData* mimeData(const QModelIndexList& indexes) const override;
			[[nodiscard]] QMap<QString, QString> searchOptions() const override;

		public slots: // NOLINT(readability-redundant-access-specifiers)
			void refreshData();
			void reverseTracks();
			void randomizeTracks();
			void sortTracks(Library::TrackSortorder sortorder);
			void jumpToNextAlbum();

		protected:
			[[nodiscard]] int itemCount() const override;
			[[nodiscard]] QString searchableString(int index, const QString& prefix) const override;

		private slots:
			void playlistChanged(int playlistIndex);
			void currentTrackChanged(int oldIndex, int newIndex);

			void coversChanged();
			void coverFound(const QPixmap& pixmap);
			void coverLookupFinished(bool success);

		private: // NOLINT(readability-redundant-access-specifiers)
			void startCoverLookup(const MetaData& track) const;
			void lookChanged();
			void refreshPlaylist(int rowCount, int columnCount);
	};

	Util::Set<int> removeDisabledRows(const Util::Set<int>& selectedRows, Model* model);
}

#endif /* PLAYLISTITEMMODEL_H */
