/* PlaylistActionMenu.h */

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

#ifndef PLAYLISTACTIONMENU_H
#define PLAYLISTACTIONMENU_H

#include <QMenu>
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"

class DynamicPlaybackChecker;
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
			ActionMenu(DynamicPlaybackChecker* dynamicPlaybackChecker, QWidget* parent = nullptr);
			~ActionMenu() override;

			void checkDynamicPlayButton();

		private slots:
			void rep1Checked(bool checked);
			void repAllChecked(bool checked);
			void shuffleChecked(bool checked);
			void changePlaylistMode();
			void playlistModeSettingChanged();
			void gaplessClicked();

		protected:
			void languageChanged() override;
	};
}

#endif // PLAYLISTACTIONMENU_H
