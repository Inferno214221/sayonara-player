/* GUI_PlaylistPreferences.h */

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


/* GUI_PlaylistPreferences.h */

#ifndef GUI_PLAYLISTPREFERENCES_H
#define GUI_PLAYLISTPREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"

UI_FWD(GUI_PlaylistPreferences)

class GUI_PlaylistPreferences :
	public Preferences::Base
{
	Q_OBJECT
	UI_CLASS_SHARED_PTR(GUI_PlaylistPreferences)

	public:
		explicit GUI_PlaylistPreferences(const QString& identifier);
		~GUI_PlaylistPreferences() override;

		bool commit() override;
		void revert() override;

		QString actionName() const override;

	protected:
		void initUi() override;
		void retranslate() override;
		void skinChanged() override;

		QString errorString() const override;

	private slots:
		void checkboxToggled(bool b);
		void chooseColorClicked();
};

#endif // GUI_PLAYLISTPREFERENCES_H
