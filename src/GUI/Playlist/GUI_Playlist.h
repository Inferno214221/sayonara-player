/* GUI_Playlist.h */

/* Copyright (C) 2011-2017 Lucio Carreras
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
 *      Author: Lucio Carreras
 */

#ifndef GUI_PLAYLIST_H_
#define GUI_PLAYLIST_H_

#include "Utils/Message/Message.h"
#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Pimpl.h"

#include "GUI/Utils/Widgets/Widget.h"

#include "Components/PlayManager/PlayState.h"
#include "Components/Playlist/PlaylistDBInterface.h"

class PlaylistView;

UI_FWD(Playlist_Window)

class GUI_Playlist :
		public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(Playlist_Window)
	PIMPL(GUI_Playlist)

public:
	explicit GUI_Playlist(QWidget *parent=nullptr);
	~GUI_Playlist();

private:
	PlaylistView* view_by_index(int idx);
	PlaylistView* current_view();

	void set_total_time_label();

	/** Overridden events **/
	void language_changed() override;
	void skin_changed() override;

	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragLeaveEvent(QDragLeaveEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;

	Message::Answer show_save_message_box(Playlist::DBInterface::SaveAsAnswer answer);

private slots:

	// triggered from playlist
	void playlist_created(PlaylistPtr pl);
	void playlist_added(PlaylistPtr pl);
	void playlist_name_changed(int pl_idx);
	void playlist_changed(int pl_idx);
	void playlist_idx_changed(int pld_idx);

	// triggered by GUI
	void tab_close_playlist_clicked(int pl_idx); // GUI_PlaylistTabs.cpp
	void tab_save_playlist_clicked(int pl_idx); // GUI_PlaylistTabs.cpp
	void tab_save_playlist_as_clicked(int pl_idx, const QString& str); // GUI_PlaylistTabs.cpp
	void tab_save_playlist_to_file_clicked(int pl_idx, const QString& filename); // GUI_PlaylistTabs.cpp
	void tab_rename_clicked(int pl_idx, const QString& str);
	void tab_delete_playlist_clicked(int pl_idx); // GUI_PlaylistTabs.cpp
	void tab_metadata_dropped(int pl_idx, const MetaDataList& v_md);
	void open_file_clicked(int pl_idx);
	void open_dir_clicked(int pl_idx);
	void delete_tracks_clicked(const IndexSet& rows);

	void check_tab_icon();
	void check_playlist_menu(PlaylistConstPtr pl);
	void check_playlist_name(PlaylistConstPtr pl);

	void double_clicked(int row);

	void add_playlist_button_pressed();

	void clear_button_pressed(int pl_idx);

	// called by playmanager
	void playstate_changed(PlayState state);
	void playlist_finished();

	void sl_show_clear_button_changed();
};

#endif /* GUI_PLAYLIST_H_ */
