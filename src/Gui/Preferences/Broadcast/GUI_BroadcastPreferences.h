/* GUI_BroadcastSetup.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef GUI_BROADCASTSETUP_H
#define GUI_BROADCASTSETUP_H

#include "Gui/Preferences/PreferenceWidget.h"

UI_FWD(GUI_BroadcastPreferences)

class GUI_BroadcastPreferences :
		public Preferences::Base
{
	Q_OBJECT
	UI_CLASS(GUI_BroadcastPreferences)

public:
	explicit GUI_BroadcastPreferences(const QString& identifier);
	~GUI_BroadcastPreferences() override;

	bool commit() override;
	void revert() override;

	QString action_name() const override;

protected:
	void init_ui() override;
	void skin_changed() override;
	void retranslate_ui() override;

private slots:
	void active_toggled(bool b);
	void port_changed(int new_val);

private:
	QString get_url_string() const;
	void refresh_url();
};

#endif // GUI_BROADCASTSETUP_H
