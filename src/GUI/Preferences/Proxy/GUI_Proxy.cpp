/* GUI_Proxy.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#include "GUI_Proxy.h"
#include "ui_GUI_Proxy.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/Crypt.h"

GUI_Proxy::GUI_Proxy(const QString& identifier) :
	Base(identifier)
{}

GUI_Proxy::~GUI_Proxy()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_Proxy::init_ui()
{
	setup_parent(this, &ui);

	connect(ui->cb_active, &QCheckBox::toggled, this, &GUI_Proxy::active_toggled);

	revert();
}

void GUI_Proxy::retranslate_ui()
{
	ui->retranslateUi(this);
}

QString GUI_Proxy::action_name() const
{
	return tr("Proxy");
}

bool GUI_Proxy::commit()
{
	_settings->set<Set::Proxy_Active>(ui->cb_active->isChecked());
	_settings->set<Set::Proxy_Username>(ui->le_username->text());

	_settings->set<Set::Proxy_Hostname>(ui->le_host->text());
	_settings->set<Set::Proxy_Port>(ui->sb_port->value());
	_settings->set<Set::Proxy_SavePw>(ui->cb_save_pw->isChecked());

	if(ui->cb_save_pw->isChecked())
	{
		QString pw = ui->le_password->text();
		QString str = Util::Crypt::encrypt(pw);

		_settings->set<Set::Proxy_Password>(str);
	}
	else {
		_settings->set<Set::Proxy_Password>(QString());
	}

	return true;
}

void GUI_Proxy::revert()
{
	bool active = _settings->get<Set::Proxy_Active>();

	ui->cb_active->setChecked(active);

	ui->le_host->setText(_settings->get<Set::Proxy_Hostname>());
	ui->sb_port->setValue(_settings->get<Set::Proxy_Port>());
	ui->le_username->setText(_settings->get<Set::Proxy_Username>());

	QString pw = Util::Crypt::decrypt(_settings->get<Set::Proxy_Password>());
	ui->le_password->setText(pw);
	ui->cb_save_pw->setChecked(_settings->get<Set::Proxy_SavePw>());

	active_toggled(active);
}

void GUI_Proxy::active_toggled(bool active)
{
	ui->le_host->setEnabled(active);
	ui->le_password->setEnabled(active);
	ui->sb_port->setEnabled(active);
	ui->le_username->setEnabled(active);
	ui->cb_save_pw->setEnabled(active);
}
