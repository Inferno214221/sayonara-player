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

#ifndef PLAYLISTITEMMODEL_H
#define PLAYLISTITEMMODEL_H

#include "Gui/Utils/SearchableWidget/SearchableModel.h"

#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Pimpl.h"

class LibraryInfoAccessor;
class CustomMimeData;
class QPixmap;

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
				CurrentPlayingRole = Qt::UserRole + 4
			};

			Model(const PlaylistPtr& playlist, LibraryInfoAccessor* libraryInfoAccessor, QObject* parent);
			~Model() override;

			[[nodiscard]] int playlistIndex() const;

			void clear();
			void removeTracks(const IndexSet& rows);
			void deleteTracks(const IndexSet& rows);

			void findTrack(int index);

			IndexSet moveTracks(const IndexSet& rows, int targetIndex);
			IndexSet moveTracksUp(const IndexSet& rows);
			IndexSet moveTracksDown(const IndexSet& rows);
			IndexSet copyTracks(const IndexSet& rows, int target_index);
			void insertTracks(const MetaDataList& tracks, int row);
			void insertTracks(const QStringList& files, int row);

			[[nodiscard]] int currentTrack() const;

			[[nodiscard]] MetaDataList metadata(const IndexSet& rows) const;

			[[nodiscard]] bool hasLocalMedia(const IndexSet& rows) const;
			void setDragIndex(int dragIndex);
			void changeRating(const IndexSet& rows, Rating rating);
			void changeTrack(int trackIndex, Seconds seconds = 0);

			void setBusy(bool b);

			[[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override;
			[[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
			bool setData(const QModelIndex& index, const QVariant& value, int role) override;
			[[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
			[[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;

			SearchableModelInterface::ExtraTriggerMap getExtraTriggers() override;
			[[nodiscard]] QMimeData* mimeData(const QModelIndexList& indexes) const override;
			QModelIndexList searchResults(const QString& searchString) override;

		public slots: // NOLINT(readability-redundant-access-specifiers)
			void refreshData();
			void reverseTracks();
			void randomizeTracks();
			void jumpToNextAlbum();

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
}

#endif /* PLAYLISTITEMMODEL_H */
