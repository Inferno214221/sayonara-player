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

#include "GUI_RemoteControlPreferences.h"
#include "Gui/Preferences/ui_GUI_RemoteControlPreferences.h"

#include "Utils/Utils.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

GUI_RemoteControlPreferences::GUI_RemoteControlPreferences(const QString& identifier) :
	Base(identifier) {}

GUI_RemoteControlPreferences::~GUI_RemoteControlPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_RemoteControlPreferences::init_ui()
{
	setup_parent(this, &ui);

	connect(ui->cb_activate, &QCheckBox::toggled, this, &GUI_RemoteControlPreferences::active_toggled);
	connect(ui->sb_port, spinbox_value_changed_int, this, &GUI_RemoteControlPreferences::port_changed);
	connect(ui->sb_discover, spinbox_value_changed_int, this, &GUI_RemoteControlPreferences::discover_port_changed);

	revert();
}

void GUI_RemoteControlPreferences::retranslate_ui()
{
	ui->retranslateUi(this);
	ui->lab_active->setText(Lang::get(Lang::Active));

	const QString tooltip = tr("If activated, Sayonara will answer an UDP request that it is remote controlable");
	ui->cb_discover->setToolTip(tooltip);
	ui->lab_discover->setToolTip(tooltip);
}


bool GUI_RemoteControlPreferences::commit()
{
	SetSetting(Set::Remote_Active, ui->cb_activate->isChecked());
	SetSetting(Set::Remote_Port, ui->sb_port->value());
	SetSetting(Set::Remote_Discoverable, ui->cb_discover->isChecked());
	SetSetting(Set::Remote_DiscoverPort, ui->sb_discover->value());

	return true;
}

void GUI_RemoteControlPreferences::revert()
{
	bool active = GetSetting(Set::Remote_Active);
	ui->cb_activate->setChecked(active);

	ui->sb_port->setEnabled(active);
	ui->sb_port->setValue(GetSetting(Set::Remote_Port));

	ui->cb_discover->setEnabled(active);
	ui->cb_discover->setChecked(GetSetting(Set::Remote_Discoverable));

	ui->sb_discover->setEnabled(active);
	ui->sb_discover->setValue(GetSetting(Set::Remote_DiscoverPort));

	refresh_url();
}


QString GUI_RemoteControlPreferences::action_name() const
{
	return tr("Remote control");
}


void GUI_RemoteControlPreferences::active_toggled(bool b)
{
	Q_UNUSED(b)
	refresh_url();

	bool active = GetSetting(Set::Remote_Active);

	ui->sb_port->setEnabled(active);
	ui->cb_discover->setEnabled(active);
	ui->sb_discover->setEnabled(active);
}

void GUI_RemoteControlPreferences::port_changed(int port)
{
	Q_UNUSED(port)

	ui->sb_discover->setValue(port + 1);
	refresh_url();
}

void GUI_RemoteControlPreferences::discover_port_changed(int port)
{
	if(port == ui->sb_port->value())
	{
		ui->sb_discover->setValue(ui->sb_port->value() + 1);
	}
}


QString GUI_RemoteControlPreferences::get_url_string()
{
	int port = ui->sb_port->value();
	const QStringList ips = Util::ip_addresses();

	QStringList ret;
	for(const QString& ip : ips)
	{
		ret << QString("%1:%2")
					.arg(ip)
					.arg(port);
	}

	return ret.join("\n");
}

void GUI_RemoteControlPreferences::refresh_url()
{
	bool active = ui->cb_activate->isChecked();

	ui->lab_url->setVisible(active);
	ui->lab_urls->setText(get_url_string());
}

bool GUI_RemoteControlPreferences::has_error() const
{
	int broadcast_port = GetSetting(Set::Broadcast_Port);

	return
		(ui->sb_port->value() == broadcast_port) ||
		(ui->sb_discover->value() == broadcast_port);
}

QString GUI_RemoteControlPreferences::error_string() const
{
	int broadcast_port = GetSetting(Set::Broadcast_Port);
	return tr("Port %1 already in use").arg(broadcast_port);
}
