/* GuiSmartPlaylists.h */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#ifndef SAYONARA_PLAYER_GUISMARTPLAYLISTS_H
#define SAYONARA_PLAYER_GUISMARTPLAYLISTS_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/GuiClass.h"
#include "Gui/Plugins/PlayerPluginBase.h"

#include <QList>

UI_FWD(GuiSmartPlaylists)

class SmartPlaylistManager;
class GuiSmartPlaylists :
	public PlayerPlugin::Base
{
	Q_OBJECT
	PIMPL(GuiSmartPlaylists)
	UI_CLASS(GuiSmartPlaylists)

	public:
		explicit GuiSmartPlaylists(SmartPlaylistManager* smartPlaylistManager, QWidget* parent = nullptr);
		~GuiSmartPlaylists() noexcept override;

		[[nodiscard]] QString name() const override;
		[[nodiscard]] QString displayName() const override;

	private slots:
		void setupPlaylists();
		void selectedIndexChanged(int index);
		void newClicked();
		void editClicked();
		void deleteClicked();

	private: // NOLINT(readability-redundant-access-specifiers)
		void retranslate() override;
		void initUi() override;

};

#endif //SAYONARA_PLAYER_GUISMARTPLAYLISTS_H
