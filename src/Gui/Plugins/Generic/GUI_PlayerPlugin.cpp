/* GUI_PlayerPlugin.cpp */

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


/* GUI_PlayerPlugin.cpp */

#include "GUI_PlayerPlugin.h"
#include "Gui/Plugins/ui_GUI_PlayerPlugin.h"
#include "Gui/Plugins/PlayerPluginBase.h"
#include "Gui/Plugins/PlayerPluginHandler.h"

struct GUI_PlayerPlugin::Private
{
	PlayerPlugin::Base* currentPlugin = nullptr;
};

GUI_PlayerPlugin::GUI_PlayerPlugin(QWidget* parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::GUI_PlayerPlugin();
	ui->setupUi(this);

	connect(ui->btnClose, &QPushButton::clicked, this, &GUI_PlayerPlugin::close);
}

GUI_PlayerPlugin::~GUI_PlayerPlugin()
{
	delete ui;
	ui = nullptr;
}

void GUI_PlayerPlugin::show(PlayerPlugin::Base* plugin)
{
	closeCurrentPlugin();
	m->currentPlugin = plugin;

	if(!plugin)
	{
		return;
	}

	bool show_title = plugin->hasTitle();

	ui->headerWidget->setVisible(show_title);
	ui->labTitle->setText(plugin->displayName());

	ui->verticalLayout->setSpacing(0);
	ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
	ui->verticalLayout->insertWidget(1, plugin);

	plugin->show();

	Widget::show();
}

void GUI_PlayerPlugin::showCurrentPlugin()
{
	PlayerPlugin::Handler* pph = PlayerPlugin::Handler::instance();
	show(pph->currentPlugin());
}

void GUI_PlayerPlugin::closeCurrentPlugin()
{
	if(m->currentPlugin)
	{
		m->currentPlugin->close();
	}

	m->currentPlugin = nullptr;
}

void GUI_PlayerPlugin::languageChanged()
{
	if(m->currentPlugin)
	{
		ui->labTitle->setText(m->currentPlugin->displayName());
	}
}

void GUI_PlayerPlugin::closeEvent(QCloseEvent* e)
{
	closeCurrentPlugin();

	Widget::closeEvent(e);
}
