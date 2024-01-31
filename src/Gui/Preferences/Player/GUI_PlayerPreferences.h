/* GUI_PlayerPreferences.h */

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

#ifndef GUI_PLAYERPREFERENCES_H
#define GUI_PLAYERPREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"

UI_FWD(GUI_PlayerPreferences)

class GUI_PlayerPreferences :
	public Preferences::Base
{
	Q_OBJECT
	PIMPL(GUI_PlayerPreferences)
	UI_CLASS_SHARED_PTR(GUI_PlayerPreferences)

	public:
		GUI_PlayerPreferences(const QString& identifier, bool canInhibitIdle);
		~GUI_PlayerPreferences() override;

		bool commit() override;
		void revert() override;

		[[nodiscard]] QString actionName() const override;

	protected:
		void initUi() override;
		void retranslate() override;

		void logLevelChanged();

	private slots:
		void showTrayIconToggled(bool b);
};

#endif // GUI_PLAYERPREFERENCES_H
