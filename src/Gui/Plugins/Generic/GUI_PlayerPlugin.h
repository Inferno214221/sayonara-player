/* GUI_PlayerPluginBase.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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


/* GUI_PlayerPluginBase.h */

#ifndef GUI_PLAYERPLUGIN_H
#define GUI_PLAYERPLUGIN_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_PlayerPlugin)

namespace PlayerPlugin
{
	class Base;
}

class GUI_PlayerPlugin :
		public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(GUI_PlayerPlugin)
	PIMPL(GUI_PlayerPlugin)

public:
	explicit GUI_PlayerPlugin(QWidget* parent = nullptr);
	~GUI_PlayerPlugin() override;

	void setContent(PlayerPlugin::Base* playerPlugin);
	void show(PlayerPlugin::Base* playerPlugin);
	void showCurrentPlugin();

private:
	void closeCurrentPlugin();

protected:
	void languageChanged() override;
	void closeEvent(QCloseEvent *e) override;
};

#endif // GUI_PLAYERPLUGIN_H
