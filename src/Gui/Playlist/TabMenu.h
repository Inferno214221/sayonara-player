
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

#ifndef PLAYLISTTABMENU_H
#define PLAYLISTTABMENU_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "MenuEntry.h"
#include "Utils/Pimpl.h"

#include <QMenu>

namespace Gui
{
	class PreferenceAction;
}

class PlaylistTabMenu :
		public Gui::WidgetTemplate<QMenu>
{
	Q_OBJECT
	PIMPL(PlaylistTabMenu)

	signals:
		void sig_delete_clicked();
		void sig_save_clicked();
		void sig_save_as_clicked();
		void sig_close_clicked();
		void sig_close_others_clicked();
		void sig_reset_clicked();
		void sig_rename_clicked();
		void sig_clear_clicked();
		void sig_open_file_clicked();
		void sig_open_dir_clicked();
		void sig_save_to_file_clicked();

	protected:
		void language_changed() override;
		void skin_changed() override;


	public:
		explicit PlaylistTabMenu(QWidget* parent=nullptr);
		~PlaylistTabMenu();

		void show_menu_items(PlaylistMenuEntries entries);
		void show_close(bool b);

		void add_preference_action(Gui::PreferenceAction* action);
};


#endif // PLAYLISTTABMENU_H
