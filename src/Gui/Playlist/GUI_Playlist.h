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

#include "Utils/Message/Message.h"
#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Pimpl.h"

#include "Gui/Utils/Widgets/Widget.h"

#include "Components/PlayManager/PlayState.h"
#include "Components/Playlist/PlaylistDBInterface.h"

namespace Playlist
{
	class View;
}

UI_FWD(Playlist_Window)

/**
 * @brief The GUI_Playlist class
 * @ingroup GuiPlaylists
 */
class GUI_Playlist :
		public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(Playlist_Window)
	PIMPL(GUI_Playlist)

public:
	explicit GUI_Playlist(QWidget* parent=nullptr);
	~GUI_Playlist() override;

private:
	Playlist::View* viewByIndex(int idx);
	Playlist::View* currentView();

	void setTotalTimeLabel();

private slots:

	// triggered from playlist
	void playlistCreated(PlaylistPtr pl);
	void playlistAdded(PlaylistPtr pl);
	void playlistNameChanged(int playlistIndex);
	void playlistChanged(int playlistIndex);
	void playlistIdxChanged(int pld_idx);

	// triggered by GUI
	void tabClosePlaylistClicked(int playlistIndex); // GUI_PlaylistTabs.cpp
	void tabSavePlaylistClicked(int playlistIndex); // GUI_PlaylistTabs.cpp
	void tabSavePlaylistAsClicked(int playlistIndex, const QString& str); // GUI_PlaylistTabs.cpp
	void tabSavePlaylistToFileClicked(int playlistIndex, const QString& filename); // GUI_PlaylistTabs.cpp
	void tabRenameClicked(int playlistIndex, const QString& str);
	void tabDeletePlaylistClicked(int playlistIndex); // GUI_PlaylistTabs.cpp
	void tabMetadataDropped(int playlistIndex, const MetaDataList& tracks);
	void tabFilesDropped(int playlistIndex, const QStringList& paths);
	void openFileClicked(int playlistIndex);
	void openDirClicked(int playlistIndex);
	void deleteTracksClicked(const IndexSet& rows);

	void checkTabIcon();
	void checkPlaylistMenu(PlaylistConstPtr pl);
	void checkPlaylistName(PlaylistConstPtr pl);

	void doubleClicked(int row);

	void addPlaylistButtonPressed();

	void clearButtonPressed(int playlistIndex);
	void bookmarkSelected(int idx, Seconds timestamp);

	// called by playmanager
	void playstateChanged(PlayState state);
	void playlistFinished();

	void showClearButtonChanged();

protected:
	void languageChanged() override;
	void skinChanged() override;

	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragLeaveEvent(QDragLeaveEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
};

#endif /* GUI_PLAYLIST_H_ */
