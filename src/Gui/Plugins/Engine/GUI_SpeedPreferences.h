/* GUI_SpeedPreferences.h */

/* Copyright (C) 2011-2023 Michael Lugmair
 *
 * This file is part of Sayonara Player
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

#ifndef SAYONARA_PLAYER_GUI_SPEEDPREFERENCES_H
#define SAYONARA_PLAYER_GUI_SPEEDPREFERENCES_H

#include "Gui/Utils/GuiClass.h"
#include "Gui/Utils/Widgets/Dialog.h"

UI_FWD(GUI_SpeedPreferences)

class GUI_SpeedPreferences :
	public Gui::Dialog
{
	Q_OBJECT
	UI_CLASS_SHARED_PTR(GUI_SpeedPreferences)

	public:
		explicit GUI_SpeedPreferences(QWidget* parent = nullptr);
		~GUI_SpeedPreferences() override;

	protected:
		void showEvent(QShowEvent* e) override;
};

#endif //SAYONARA_PLAYER_GUI_SPEEDPREFERENCES_H
