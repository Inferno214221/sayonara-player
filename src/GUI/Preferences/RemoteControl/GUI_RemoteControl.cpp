/* GUIRemoteControl.cpp

 * Copyright (C) 2011-2019 Lucio Carreras
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
 * created by Lucio Carreras,
 * Sep 3, 2012
 *
 */

#include "GUI_RemoteControl.h"
#include "GUI/Preferences/ui_GUI_RemoteControl.h"

#include "Utils/Utils.h"
#include "Utils/Language.h"
#include "Utils/Settings/Settings.h"


GUI_RemoteControl::GUI_RemoteControl(const QString& identifier) :
	Base(identifier) {}

GUI_RemoteControl::~GUI_RemoteControl()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_RemoteControl::init_ui()
{
	setup_parent(this, &ui);

	revert();

	connect(ui->cb_activate, &QCheckBox::toggled, this, &GUI_RemoteControl::active_toggled);
	connect(ui->sb_port, spinbox_value_changed_int, this, &GUI_RemoteControl::port_changed);
}

void GUI_RemoteControl::retranslate_ui()
{
	ui->retranslateUi(this);
	ui->lab_active->setText(Lang::get(Lang::Active));
}


bool GUI_RemoteControl::commit()
{
	SetSetting(Set::Remote_Active, ui->cb_activate->isChecked());
	SetSetting(Set::Remote_Port, ui->sb_port->value());

	return true;
}

void GUI_RemoteControl::revert()
{
	ui->cb_activate->setChecked(GetSetting(Set::Remote_Active));
	ui->sb_port->setValue(GetSetting(Set::Remote_Port));

	refresh_url();
}


QString GUI_RemoteControl::action_name() const
{
	return tr("Remote control");
}


void GUI_RemoteControl::active_toggled(bool b)
{
	Q_UNUSED(b)
	refresh_url();
}

void GUI_RemoteControl::port_changed(int port)
{
	Q_UNUSED(port)
	refresh_url();
}


QString GUI_RemoteControl::get_url_string()
{
	int port = ui->sb_port->value();
	QStringList ips = Util::ip_addresses();

	QStringList ret;
	for(const QString& ip : ips){
		QString str = QString("http://") + ip + ":" + QString::number(port);
		ret << str;
	}

	return ret.join("; ");
}

void GUI_RemoteControl::refresh_url()
{
	bool active = ui->cb_activate->isChecked();

	ui->le_url->setVisible(active);
	ui->lab_url->setVisible(active);
	ui->le_url->setText(get_url_string());
}
