
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

#ifndef PLAYLISTTABMENU_H
#define PLAYLISTTABMENU_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "PlaylistMenuEntry.h"
#include "Utils/Pimpl.h"

#include <QMenu>

namespace Gui
{
	class PreferenceAction;
}

namespace Playlist
{
	/**
	 * @brief The PlaylistTabMenu class
	 * @ingroup GuiPlaylists
	 */
	class TabMenu :
		public Gui::WidgetTemplate<QMenu>
	{
		Q_OBJECT
		PIMPL(TabMenu)

		signals:
			void sigDeleteClicked();
			void sigSaveClicked();
			void sigSaveAsClicked();
			void sigCloseClicked();
			void sigCloseOthersClicked();
			void sigResetClicked();
			void sigRenameClicked();
			void sigClearClicked();
			void sigOpenFileClicked();
			void sigOpenDirClicked();
			void sigSaveToFileClicked();
			void sigLockTriggered(bool b);

		public:
			explicit TabMenu(QWidget* parent = nullptr);
			~TabMenu() override;

			void showMenuItems(MenuEntries entries);
			void showClose(bool b);

			void addPreferenceAction(Gui::PreferenceAction* action);

		protected:
			void languageChanged() override;
			void skinChanged() override;
	};
}

#endif // PLAYLISTTABMENU_H
