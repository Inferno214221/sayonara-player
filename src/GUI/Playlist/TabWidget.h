/* PlaylistTabWidget.h */

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

#ifndef PLAYLISTTABWIDGET_H
#define PLAYLISTTABWIDGET_H

#include "MenuEntry.h"
#include "Utils/Pimpl.h"

#include <QTabWidget>

class PlaylistTabWidget :
		public QTabWidget
{
	Q_OBJECT
	PIMPL(PlaylistTabWidget)

signals:
	void sig_open_file(int tab_idx);
	void sig_open_dir(int tab_idx);
	void sig_tab_reset(int tab_idx);
	void sig_tab_save(int tab_idx);
	void sig_tab_save_as(int tab_idx, const QString& name);
	void sig_tab_save_to_file(int tab_idx, const QString& filename);
	void sig_tab_rename(int tab_idx, const QString& name);
	void sig_tab_delete(int tab_idx);
	void sig_tab_clear(int tab_idx);
	void sig_add_tab_clicked();
	void sig_metadata_dropped(int tab_idx, const MetaDataList& v_md);

public:
	explicit PlaylistTabWidget(QWidget* parent=nullptr);
	~PlaylistTabWidget();

	void show_menu_items(PlaylistMenuEntries actions);

	void removeTab(int index);
	void addTab(QWidget* widget, const QIcon& icon, const QString& label);
	void addTab(QWidget* widget, const QString& label);
	void insertTab(int index, QWidget* widget, const QString& label);
	void insertTab(int index, QWidget* widget, const QIcon& icon, const QString& label);

	bool was_drag_from_playlist() const;
	int get_drag_origin_tab() const;

private:
	void check_last_tab();
};

#endif // PLAYLISTTABWIDGET_H
