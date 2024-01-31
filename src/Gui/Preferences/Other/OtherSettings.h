/* OtherSettings.h, (Created on 08.01.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef SAYONARA_PLAYER_OTHERSETTINGS_H
#define SAYONARA_PLAYER_OTHERSETTINGS_H

#include "Gui/Preferences/PreferenceWidget.h"

UI_FWD(GUI_OtherSettings)

class GUI_OtherSettings :
	public Preferences::Base
{
	Q_OBJECT
	UI_CLASS_SHARED_PTR(GUI_OtherSettings)

	public:
		explicit GUI_OtherSettings(const QString& identifier);
		~GUI_OtherSettings() override;

		[[nodiscard]] QString actionName() const override;

		bool commit() override;

		void revert() override;

		void initUi() override;

		void retranslate() override;

		bool hasError() const override;

		QString errorString() const override;
};

#endif //SAYONARA_PLAYER_OTHERSETTINGS_H
