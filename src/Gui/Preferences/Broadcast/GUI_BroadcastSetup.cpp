/* GUI_BroadcastSetup.cpp */

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

#include "GUI_BroadcastSetup.h"
#include "Gui/ui_GUI_BroadcastSetup.h"

#include "Gui/Utils/Widgets/WidgetTemplate.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"


GUI_BroadcastSetup::GUI_BroadcastSetup(const QString& identifier) :
	Base(identifier) {}

GUI_BroadcastSetup::~GUI_BroadcastSetup()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_BroadcastSetup::init_ui()
{
	setup_parent(this, &ui);

	revert();

	connect(ui->cb_active, &QCheckBox::toggled, this, &GUI_BroadcastSetup::active_toggled);
	connect(ui->sb_port, spinbox_value_changed_int, this, &GUI_BroadcastSetup::port_changed);
}

bool GUI_BroadcastSetup::commit()
{
	bool new_active = ui->cb_active->isChecked();
	bool new_prompt = ui->cb_prompt->isChecked();
	int new_port = ui->sb_port->value();

	bool old_active = GetSetting(Set::Broadcast_Active);
	bool old_prompt = GetSetting(Set::Broadcast_Prompt);
	int old_port = GetSetting(Set::Broadcast_Port);

	if(old_active != new_active){
		SetSetting(Set::Broadcast_Active, new_active);
	}

	if(old_prompt != new_prompt){
		SetSetting(Set::Broadcast_Prompt, new_prompt);
	}

	if(old_port != new_port){
		SetSetting(Set::Broadcast_Port, new_port);
	}

	return true;
}


void GUI_BroadcastSetup::revert()
{
	bool active = GetSetting(Set::Broadcast_Active);

	ui->cb_active->setChecked( active );
	ui->cb_prompt->setChecked( GetSetting(Set::Broadcast_Prompt) );
	ui->sb_port->setValue( GetSetting(Set::Broadcast_Port) );
	ui->le_url->setVisible(active);
	ui->lab_url_title->setVisible(active);

	refresh_url();
}


void GUI_BroadcastSetup::skin_changed()
{
	if(!is_ui_initialized()){
		return;
	}
}

void GUI_BroadcastSetup::retranslate_ui()
{
	ui->retranslateUi(this);
	ui->lab_activate->setText(Lang::get(Lang::Active));
	ui->lab_url_title->setText(Lang::get(Lang::StreamUrl));
}


QString GUI_BroadcastSetup::action_name() const
{
	return Lang::get(Lang::Broadcast);
}


void GUI_BroadcastSetup::active_toggled(bool b)
{
	Q_UNUSED(b);
	refresh_url();
}

void GUI_BroadcastSetup::port_changed(int new_val)
{
	Q_UNUSED(new_val);
	refresh_url();
}

QString GUI_BroadcastSetup::get_url_string() const
{
	int port = ui->sb_port->value();
	QStringList ips = Util::ip_addresses();

	QStringList ret;
	for(const QString& ip : ips){
		QString str = QString("http://") + ip + ":" + QString::number(port) + "/playlist.m3u";
		ret << str;
	}

	return ret.join("; ");
}

void GUI_BroadcastSetup::refresh_url()
{
	bool active = ui->cb_active->isChecked();

	ui->le_url->setVisible(active);
	ui->lab_url_title->setVisible(active);
	ui->le_url->setText(get_url_string());
}


