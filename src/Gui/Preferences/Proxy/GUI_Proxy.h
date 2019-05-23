/* GUI_Proxy.h */

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



#ifndef GUI_PROXY_H
#define GUI_PROXY_H

#include "Interfaces/PreferenceDialog/PreferenceWidget.h"

UI_FWD(GUI_Proxy)

class GUI_Proxy :
		public Preferences::Base
{
	Q_OBJECT
	UI_CLASS(GUI_Proxy)

public:
	explicit GUI_Proxy(const QString& identifier);
	~GUI_Proxy();

	// PreferenceInterface interface
protected:
	void init_ui() override;
	void retranslate_ui() override;

public:
	QString action_name() const override;
	bool commit() override;
	void revert() override;

private slots:
	void active_toggled(bool active);
};



#endif // GUI_PROXY_H
