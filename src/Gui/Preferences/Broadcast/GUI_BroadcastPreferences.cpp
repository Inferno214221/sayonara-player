/* GUI_BroadcastSetup.cpp */

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

#include "GUI_BroadcastPreferences.h"
#include "Gui/Preferences/ui_GUI_BroadcastPreferences.h"

#include "Gui/Utils/Widgets/WidgetTemplate.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"


GUI_BroadcastPreferences::GUI_BroadcastPreferences(const QString& identifier) :
	Base(identifier) {}

GUI_BroadcastPreferences::~GUI_BroadcastPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_BroadcastPreferences::initUi()
{
	setupParent(this, &ui);

	revert();

	connect(ui->cbActive, &QCheckBox::toggled, this, &GUI_BroadcastPreferences::activeToggled);
	connect(ui->sbPort, spinbox_value_changed_int, this, &GUI_BroadcastPreferences::portChanged);
}

bool GUI_BroadcastPreferences::commit()
{
	bool new_active = ui->cbActive->isChecked();
	bool new_prompt = ui->cbPrompt->isChecked();
	int new_port = ui->sbPort->value();

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


void GUI_BroadcastPreferences::revert()
{
	bool active = GetSetting(Set::Broadcast_Active);

	ui->cbActive->setChecked( active );
	ui->cbPrompt->setChecked( GetSetting(Set::Broadcast_Prompt) );
	ui->sbPort->setValue( GetSetting(Set::Broadcast_Port) );
	ui->leUrl->setVisible(active);
	ui->labUrlTitle->setVisible(active);

	refreshUrl();
}


void GUI_BroadcastPreferences::skinChanged()
{
	if(!isUiInitialized()){
		return;
	}
}

void GUI_BroadcastPreferences::retranslate()
{
	ui->retranslateUi(this);

	ui->labActive->setText(Lang::get(Lang::Active));
	ui->labUrlTitle->setText(Lang::get(Lang::StreamUrl));
}


QString GUI_BroadcastPreferences::actionName() const
{
	return Lang::get(Lang::Broadcast);
}


void GUI_BroadcastPreferences::activeToggled(bool b)
{
	Q_UNUSED(b);
	refreshUrl();
}

void GUI_BroadcastPreferences::portChanged(int new_val)
{
	Q_UNUSED(new_val);
	refreshUrl();
}

QString GUI_BroadcastPreferences::urlString() const
{
	int port = ui->sbPort->value();
	QStringList ips = Util::ipAddresses();

	QStringList ret;
	for(const QString& ip : ips)
	{
		QString str = QString("http://") + ip + ":" + QString::number(port) + "/playlist.m3u";
		ret << str;
	}

	return ret.join("; ");
}

void GUI_BroadcastPreferences::refreshUrl()
{
	bool active = ui->cbActive->isChecked();

	ui->leUrl->setVisible(active);
	ui->leUrl->setText(urlString());
	ui->labUrlTitle->setVisible(active);
}

bool GUI_BroadcastPreferences::hasError() const
{
	int port = ui->sbPort->value();

	return
		(port == GetSetting(Set::Remote_Port)) ||
		(port == GetSetting(Set::Remote_DiscoverPort));
}

QString GUI_BroadcastPreferences::errorString() const
{
	return tr("Port %1 already in use").arg(ui->sbPort->value());
}
