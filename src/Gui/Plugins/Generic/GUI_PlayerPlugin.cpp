/* GUI_PlayerPlugin.cpp */

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


/* GUI_PlayerPlugin.cpp */

#include "GUI_PlayerPlugin.h"
#include "Gui/Plugins/ui_GUI_PlayerPlugin.h"
#include "Gui/Plugins/PlayerPluginBase.h"
#include "Gui/Plugins/PlayerPluginHandler.h"

struct GUI_PlayerPlugin::Private
{
	PlayerPlugin::Base* current_plugin=nullptr;
};

GUI_PlayerPlugin::GUI_PlayerPlugin(QWidget *parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::GUI_PlayerPlugin();
	ui->setupUi(this);

	connect(ui->btn_close, &QPushButton::clicked, this, &GUI_PlayerPlugin::close);
}

GUI_PlayerPlugin::~GUI_PlayerPlugin()
{
	delete ui; ui=nullptr;
}

void GUI_PlayerPlugin::show(PlayerPlugin::Base* player_plugin)
{
	close_current_plugin();
	m->current_plugin = player_plugin;

	if(!player_plugin){
		return;
	}

	bool show_title = player_plugin->is_title_shown();

	ui->header_widget->setVisible(show_title);
	ui->lab_title->setText(player_plugin->get_display_name());

	ui->verticalLayout->setSpacing(0);
	ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
	ui->verticalLayout->insertWidget(1, player_plugin);

	player_plugin->show();

	Widget::show();
}

void GUI_PlayerPlugin::show_current_plugin()
{
	PlayerPlugin::Handler* pph = PlayerPlugin::Handler::instance();
	show(pph->current_plugin());
}

void GUI_PlayerPlugin::close_current_plugin()
{
	if(m->current_plugin){
		m->current_plugin->close();
	}

	m->current_plugin = nullptr;
}

void GUI_PlayerPlugin::language_changed()
{
	if(m->current_plugin){
		ui->lab_title->setText(m->current_plugin->get_display_name());
	}
}

void GUI_PlayerPlugin::closeEvent(QCloseEvent* e)
{
	close_current_plugin();

	Widget::closeEvent(e);
}
