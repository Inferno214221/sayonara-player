/* PlaylistTabBar.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef PLAYLISTTABBAR_H
#define PLAYLISTTABBAR_H

#include "PlaylistMenuEntry.h"
#include "Utils/Pimpl.h"

#include <QTabBar>

namespace Playlist
{
	/**
	 * @brief The PlaylistTabBar class
	 * @ingroup GuiPlaylists
	 */
	class TabBar :
			public QTabBar
	{
		Q_OBJECT
		PIMPL(TabBar)

	signals:
		void sig_open_file(int tab_idx);
		void sig_open_dir(int tab_idx);

		void sig_tab_reset(int tab_idx);
		void sig_tab_save(int tab_idx);
		void sig_tab_save_as(int tab_idx, const QString& name);
		void sig_tab_save_to_file(int tab_idx, const QString& filename);
		void sig_tab_rename(int tab_idx, const QString& name);
		void sig_tab_clear(int tab_idx);

		void sig_tab_delete(int tab_idx);
		void sig_cur_idx_changed(int tab_idx);
		void sig_add_tab_clicked();
		void sig_metadata_dropped(int tab_idx, const MetaDataList& v_md);
		void sig_files_dropped(int tab_idx, const QStringList& files);


	public:
		explicit TabBar(QWidget *parent=nullptr);
		~TabBar() override;

		void show_menu_items(MenuEntries entries);
		void setTabsClosable(bool b);

		bool was_drag_from_playlist() const;
		int get_drag_origin_tab() const;

	private:
		void mousePressEvent(QMouseEvent* e) override;
		void wheelEvent(QWheelEvent* e) override;
		void dragEnterEvent(QDragEnterEvent* e) override;
		void dragMoveEvent(QDragMoveEvent* e) override;
		void dragLeaveEvent(QDragLeaveEvent* e) override;
		void dropEvent(QDropEvent* e) override;

		void init_shortcuts();

	private slots:
		void open_file_pressed();
		void open_dir_pressed();
		void reset_pressed();
		void save_pressed();
		void save_as_pressed();
		void save_to_file_pressed();
		void clear_pressed();
		void delete_pressed();
		void close_pressed();
		void close_others_pressed();
		void rename_pressed();
	};
}

#endif // PLAYLISTTABBAR_H