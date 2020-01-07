/* PlaylistActionMenu.h */

/* Copyright (C) 2011-2020 Lucio Carreras
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

#ifndef PLAYLISTACTIONMENU_H
#define PLAYLISTACTIONMENU_H

#include <QMenu>
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"

namespace Playlist
{
	/**
	 * @brief The PlaylistActionMenu class
	 * @ingroup GuiPlaylists
	 */
	class ActionMenu :
		public Gui::WidgetTemplate<QMenu>
	{
		Q_OBJECT
		PIMPL(ActionMenu)

	public:
		ActionMenu(QWidget* parent=nullptr);
		~ActionMenu() override;

		void check_dynamic_play_button();

	private slots:
		void rep1_checked(bool checked);
		void rep_all_checked(bool checked);
		void shuffle_checked(bool checked);
		void playlist_mode_changed();
		void gapless_clicked();

		void language_changed() override;

		void s_playlist_mode_changed();
	};
}

#endif // PLAYLISTACTIONMENU_H
