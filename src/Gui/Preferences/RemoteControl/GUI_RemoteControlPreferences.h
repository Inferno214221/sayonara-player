/* GUI_RemoteControlPreferences.h

 * Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Michael Lugmair (Lucio Carreras),
 * Sep 3, 2012
 *
 */

#ifndef GUI_REMOTECONTROL_PREFERENCES_H
#define GUI_REMOTECONTROL_PREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"

UI_FWD(GUI_RemoteControlPreferences)

class GUI_RemoteControlPreferences :
	public Preferences::Base
{
	Q_OBJECT
	UI_CLASS_SHARED_PTR(GUI_RemoteControlPreferences)

	public:
		explicit GUI_RemoteControlPreferences(const QString& identifier);
		~GUI_RemoteControlPreferences() override;

		bool commit() override;
		void revert() override;

		[[nodiscard]] QString actionName() const override;
		[[nodiscard]] bool hasError() const override;
		[[nodiscard]] QString errorString() const override;

	protected:
		void initUi() override;
		void retranslate() override;

	private slots:
		void activeToggled(bool b);
		void portChanged(int port);
		void discoverPortChanged(int port);

	private:
		void refreshUrl();
};

#endif /* GUI_REMOTECONTROL_PREFERENCES_H */
