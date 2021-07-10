/* PlaylistView.h */

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
 * PlaylistView.h
 *
 *  Created on: Jun 27, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef PLAYLISTVIEW_H_
#define PLAYLISTVIEW_H_

#include "Gui/Utils/SearchableWidget/SearchableView.h"
#include "Gui/Utils/Widgets/Dragable.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"

#include "Gui/InfoDialog/InfoDialogContainer.h"

#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/MetaData/MetaDataFwd.h"
#include "Utils/Pimpl.h"

class QPoint;
class DynamicPlaybackChecker;
class PlaylistCreator;

namespace Playlist
{
	/**
	 * @brief The PlaylistView class
	 * @ingroup GuiPlaylists
	 */
	class View :
			public SearchableTableView,
			public InfoDialogContainer,
			private Gui::Dragable
	{
		Q_OBJECT
		PIMPL(View)

		signals:
			void sigDoubleClicked(int row);
			void sigBookmarkPressed(int trackIdx, Seconds timestamp);

		public:
			View(PlaylistCreator* playlistCreator, PlaylistPtr playlist, DynamicPlaybackChecker* dynamicPlaybackChecker, QWidget* parent=nullptr);
			~View() override;

			void dropEventFromOutside(QDropEvent* event);
			void removeSelectedRows();

		public slots:
			void gotoRow(int row);

		protected:
			MD::Interpretation metadataInterpretation() const override;
			MetaDataList infoDialogData() const override;
            QWidget* getParentWidget() override;

			int mapModelIndexToIndex(const QModelIndex& idx) const override;
			ModelIndexRange mapIndexToModelIndexes(int idx) const override;

			void skinChanged() override;

			/**
			 * @brief we start the drag action, all lines has to be cleared
			 * @param the event
			 */
			void dragLeaveEvent(QDragLeaveEvent* event) override;
			void dragEnterEvent(QDragEnterEvent* event) override;
			void dragMoveEvent(QDragMoveEvent* event) override;
			void dropEvent(QDropEvent* event) override;
			void mousePressEvent(QMouseEvent* event) override;
			void mouseDoubleClickEvent(QMouseEvent* event) override;
			void keyPressEvent(QKeyEvent* event) override;
			bool viewportEvent(QEvent* event) override;
			void contextMenuEvent(QContextMenuEvent* e) override;

		private slots:
			void clear();
			void refresh();
			void asyncDropFinished();
			void ratingChanged(Rating rating);
			void columnsChanged();
			void showRatingChanged();
			void findTrackTriggered();
			void bookmarkTriggered(Seconds timestamp);
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
			void initShortcuts();
			void handleDrop(QDropEvent* event);
	};
}

#endif /* PlaylistView_H_ */
