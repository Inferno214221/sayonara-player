/* GUI_Playlist.h */

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
 * GUI_Playlist.h
 *
 *  Created on: Apr 6, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef GUI_PLAYLIST_H_
#define GUI_PLAYLIST_H_

#include "Gui/Utils/Widgets/Widget.h"

#include "Components/Playlist/PlaylistDBInterface.h"

#include "Utils/Message/Message.h"
#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Pimpl.h"

namespace Playlist
{
	class Handler;
	class View;
}

class PlayManager;
class DynamicPlaybackChecker;

UI_FWD(PlaylistWindow)

/**
 * @brief The GUI_Playlist class
 * @ingroup GuiPlaylists
 */
class GUI_Playlist :
	public Gui::Widget
{
	Q_OBJECT
	PIMPL(GUI_Playlist)

	public:
		explicit GUI_Playlist(QWidget* parent = nullptr);
		~GUI_Playlist() override;

		void init(Playlist::Handler* playlistHandler, PlayManager* playManager,
		          DynamicPlaybackChecker* dynamicPlaybackChecker);

	private:
		void initToolButton();

	private slots:

		// triggered from playlist
		void playlistAdded(int playlistIndex);
		void playlistNameChanged(int playlistIndex);
		void playlistChanged(int playlistIndex);
		void playlistIdxChanged(int playlistIndex);
		void playlistClosed(int playlistIndex);

		// triggered by GUI
		void tabSavePlaylistClicked(int playlistIndex); // GUI_PlaylistTabs.cpp
		void tabSavePlaylistAsClicked(int playlistIndex, const QString& newName); // GUI_PlaylistTabs.cpp
		void tabSavePlaylistToFileClicked(int playlistIndex, const QString& filename); // GUI_PlaylistTabs.cpp
		void tabRenameClicked(int playlistIndex, const QString& newName);
		void tabResetClicked(int playlistIndex);
		void tabDeletePlaylistClicked(int playlistIndex); // GUI_PlaylistTabs.cpp
		void tabMetadataDropped(int playlistIndex, const MetaDataList& tracks);
		void tabFilesDropped(int playlistIndex, const QStringList& paths);
		void openFileClicked(int playlistIndex, const QStringList& files);
		void openDirClicked(int playlistIndex, const QString& dir);

		void checkTabIcon();

		void doubleClicked(int row);

		void clearButtonPressed(int playlistIndex);
		void bookmarkSelected(int trackIndex, Seconds timestamp);

		void showClearButtonChanged();
		void showBottomBarChanged();

	protected:
		void languageChanged() override;
		void skinChanged() override;

		void dragEnterEvent(QDragEnterEvent* event) override;
		void dragLeaveEvent(QDragLeaveEvent* event) override;
		void dropEvent(QDropEvent* event) override;
		void dragMoveEvent(QDragMoveEvent* event) override;

	private:
		std::shared_ptr<Ui::PlaylistWindow> ui;
};

#endif /* GUI_PLAYLIST_H_ */
