/* GUI_ProxyPreferences.h */

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

#ifndef GUI_PROXY_PREFERENCES_H
#define GUI_PROXY_PREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"

UI_FWD(GUI_ProxyPreferences)

class GUI_ProxyPreferences :
	public Preferences::Base
{
	Q_OBJECT
	UI_CLASS_SHARED_PTR(GUI_ProxyPreferences)

	public:
		explicit GUI_ProxyPreferences(const QString& identifier);
		~GUI_ProxyPreferences() override;
		
	public:
		[[nodiscard]] QString actionName() const override;
		bool commit() override;
		void revert() override;

	protected:
		void initUi() override;
		void retranslate() override;

	private slots:
		void activeToggled(bool active);
		void autosearchClicked();
};

#endif // GUI_ProxyPreferences_H
