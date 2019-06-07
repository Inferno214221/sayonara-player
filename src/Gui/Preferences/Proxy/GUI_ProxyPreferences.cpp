/* GUI_ProxyPreferences.cpp */

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

#include "GUI_ProxyPreferences.h"
#include "ui_GUI_ProxyPreferences.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/Crypt.h"

GUI_ProxyPreferences::GUI_ProxyPreferences(const QString& identifier) :
	Base(identifier)
{}

GUI_ProxyPreferences::~GUI_ProxyPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_ProxyPreferences::init_ui()
{
	setup_parent(this, &ui);

	connect(ui->cb_active, &QCheckBox::toggled, this, &GUI_ProxyPreferences::active_toggled);

	revert();
}

void GUI_ProxyPreferences::retranslate_ui()
{
	ui->retranslateUi(this);
}

QString GUI_ProxyPreferences::action_name() const
{
	return tr("Proxy");
}

bool GUI_ProxyPreferences::commit()
{
	SetSetting(Set::Proxy_Active, ui->cb_active->isChecked());
	SetSetting(Set::Proxy_Username, ui->le_username->text());

	SetSetting(Set::Proxy_Hostname, ui->le_host->text());
	SetSetting(Set::Proxy_Port, ui->sb_port->value());
	SetSetting(Set::Proxy_SavePw, ui->cb_save_pw->isChecked());

	if(ui->cb_save_pw->isChecked())
	{
		QString pw = ui->le_password->text();
		QString str = Util::Crypt::encrypt(pw);

		SetSetting(Set::Proxy_Password, str);
	}
	else {
		SetSetting(Set::Proxy_Password, QString());
	}

	return true;
}

void GUI_ProxyPreferences::revert()
{
	bool active = GetSetting(Set::Proxy_Active);

	ui->cb_active->setChecked(active);

	ui->le_host->setText(GetSetting(Set::Proxy_Hostname));
	ui->sb_port->setValue(GetSetting(Set::Proxy_Port));
	ui->le_username->setText(GetSetting(Set::Proxy_Username));

	QString pw = Util::Crypt::decrypt(GetSetting(Set::Proxy_Password));
	ui->le_password->setText(pw);
	ui->cb_save_pw->setChecked(GetSetting(Set::Proxy_SavePw));

	active_toggled(active);
}

void GUI_ProxyPreferences::active_toggled(bool active)
{
	ui->le_host->setEnabled(active);
	ui->le_password->setEnabled(active);
	ui->sb_port->setEnabled(active);
	ui->le_username->setEnabled(active);
	ui->cb_save_pw->setEnabled(active);
}
