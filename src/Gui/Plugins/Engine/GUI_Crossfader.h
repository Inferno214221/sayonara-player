/* GUI_Crossfader.h */

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

#ifndef GUI_CROSSFADER_H
#define GUI_CROSSFADER_H

#include "Gui/Plugins/PlayerPluginBase.h"

UI_FWD(GUI_Crossfader)

class GUI_Crossfader :
		public PlayerPlugin::Base
{
	Q_OBJECT
	UI_CLASS(GUI_Crossfader)

public:
	explicit GUI_Crossfader(QWidget *parent=nullptr);
	virtual ~GUI_Crossfader();

	QString get_name() const override;
	QString get_display_name() const override;

private slots:
	void slider_changed(int val);
	void crossfader_active_changed(bool b);
	void gapless_active_changed(bool b);

	void retranslate_ui() override;
	void init_ui() override;
};

#endif // GUI_CROSSFADER_H
