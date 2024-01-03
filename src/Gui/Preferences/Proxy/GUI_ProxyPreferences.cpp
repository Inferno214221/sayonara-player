/* GUI_ProxyPreferences.cpp */

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

#include "GUI_ProxyPreferences.h"
#include "ui_GUI_ProxyPreferences.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/Crypt.h"
#include "Utils/WebAccess/Proxy.h"

GUI_ProxyPreferences::GUI_ProxyPreferences(const QString& identifier) :
	Base(identifier) {}

GUI_ProxyPreferences::~GUI_ProxyPreferences()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_ProxyPreferences::initUi()
{
	setupParent(this, &ui);

	connect(ui->cbActive, &QCheckBox::toggled, this, &GUI_ProxyPreferences::activeToggled);
	connect(ui->btnAutosearch, &QPushButton::clicked, this, &GUI_ProxyPreferences::autosearchClicked);

	revert();
}

void GUI_ProxyPreferences::retranslate()
{
	ui->retranslateUi(this);
}

QString GUI_ProxyPreferences::actionName() const
{
	return tr("Proxy");
}

bool GUI_ProxyPreferences::commit()
{
	SetSetting(Set::Proxy_Active, ui->cbActive->isChecked());
	SetSetting(Set::Proxy_Username, ui->leUsername->text());

	SetSetting(Set::Proxy_Hostname, ui->leHost->text());
	SetSetting(Set::Proxy_Port, ui->sbPort->value());
	SetSetting(Set::Proxy_SavePw, ui->cbSavePassword->isChecked());

	if(ui->cbSavePassword->isChecked())
	{
		QString pw = ui->lePassword->text();
		QString str = Util::Crypt::encrypt(pw);

		SetSetting(Set::Proxy_Password, str);
	}
	else
	{
		SetSetting(Set::Proxy_Password, QString());
	}

	Proxy::setProxy();

	return true;
}

void GUI_ProxyPreferences::revert()
{
	bool active = GetSetting(Set::Proxy_Active);

	ui->cbActive->setChecked(active);

	ui->leHost->setText(GetSetting(Set::Proxy_Hostname));
	ui->sbPort->setValue(GetSetting(Set::Proxy_Port));
	ui->leUsername->setText(GetSetting(Set::Proxy_Username));

	QString pw = Util::Crypt::decrypt(GetSetting(Set::Proxy_Password));
	ui->lePassword->setText(pw);
	ui->cbSavePassword->setChecked(GetSetting(Set::Proxy_SavePw));

	activeToggled(active);
}

void GUI_ProxyPreferences::activeToggled(bool active)
{
	ui->leHost->setEnabled(active);
	ui->lePassword->setEnabled(active);
	ui->sbPort->setEnabled(active);
	ui->leUsername->setEnabled(active);
	ui->cbSavePassword->setEnabled(active);
}

void GUI_ProxyPreferences::autosearchClicked()
{
	QString hostname = Proxy::envHostname();
	int port = Proxy::envPort();

	if(!hostname.isEmpty())
	{
		ui->cbActive->setChecked(true);
		activeToggled(true);
	}

	ui->leHost->setText(hostname);
	ui->sbPort->setValue(port);
}
