/* PlaylistView.h */

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
 * PlaylistView.h
 *
 *  Created on: Jun 27, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef SAYONARA_PLAYER_PLAYLIST_VIEW_H
#define SAYONARA_PLAYER_PLAYLIST_VIEW_H

#include "Gui/InfoDialog/InfoDialogContainer.h"
#include "Gui/Utils/SearchableWidget/SearchableView.h"
#include "Gui/Utils/SearchableWidget/SelectionView.h"
#include "Gui/Utils/Widgets/Dragable.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/MetaData/MetaDataFwd.h"
#include "Utils/Pimpl.h"
#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/typedefs.h"

class QPoint;
class DynamicPlaybackChecker;

namespace Library
{
	class InfoAccessor;
}

namespace Playlist
{
	class ContextMenu;
	class View :
		public SearchableTableView,
		public InfoDialogContainer,
		private Gui::Dragable
	{
		Q_OBJECT
		PIMPL(View)

		public:
			View(const PlaylistPtr& playlist, DynamicPlaybackChecker* dynamicPlaybackChecker,
			     Library::InfoAccessor* libraryAccessor, QWidget* parent);
			~View() override;

			void dropEventFromOutside(QDropEvent* event);
			void removeSelectedRows();

			[[nodiscard]] bool isLocked() const;
			void setLocked(bool b);
			void triggerResult() override;
			[[nodiscard]] QMap<QString, QString> commands() const override;
			void runCommand(const QString& command) override;
			void gotoRow(int row);

		protected:
			[[nodiscard]] SearchModel* searchModel() const override;

			[[nodiscard]] MD::Interpretation metadataInterpretation() const override;
			[[nodiscard]] MetaDataList infoDialogData() const override;
			[[nodiscard]] QWidget* getParentWidget() override;

			[[nodiscard]] int mapModelIndexToIndex(const QModelIndex& idx) const override;
			[[nodiscard]] ModelIndexRange mapIndexToModelIndexes(int index) const override;

			void skinChanged() override;

			void dragLeaveEvent(QDragLeaveEvent* event) override;
			void dragEnterEvent(QDragEnterEvent* event) override;
			void dragMoveEvent(QDragMoveEvent* event) override;
			void dropEvent(QDropEvent* event) override;
			void mouseDoubleClickEvent(QMouseEvent* event) override;
			bool viewportEvent(QEvent* event) override;
			void contextMenuEvent(QContextMenuEvent* e) override;

		private slots:
			void clear();
			void refresh();
			void asyncDropFinished();
			void ratingChanged(Rating rating);
			void columnsChanged();
			void showRatingChanged();
			void bookmarkTriggered(Seconds timestamp);
			void sortingTriggered(Library::TrackSortorder sortOrder);
			void moveSelectedRowsUp();
			void moveSelectedRowsDown();
			void playSelectedTrack();
			void jumpToCurrentTrack();
			void playlistBusyChanged(bool isBusy);
			void currentScannedFileChanged(const QString& currentFile);
			void currentTrackChanged(int index);
			void deleteSelectedTracks();

		private:
			void initContextMenu();
			void handleDrop(QDropEvent* event);
	};
}

#endif /* SAYONARA_PLAYER_PLAYLIST_VIEW_H */
