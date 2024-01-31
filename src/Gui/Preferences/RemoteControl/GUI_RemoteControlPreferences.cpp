/* GUIRemoteControl.cpp

 * Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
 * created by Michael Lugmair (Lucio Carreras),
 * Sep 3, 2012
 *
 */

#include "GUI_RemoteControlPreferences.h"
#include "Gui/Preferences/ui_GUI_RemoteControlPreferences.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

namespace
{
	QString getUrlString(const int port)
	{
		const auto ips = Util::ipAddresses();

		QStringList result;
		Util::Algorithm::transform(ips, result, [&](const auto& ip) {
			return QString("%1:%2")
				.arg(ip)
				.arg(port);
		});

		return result.join("\n");
	}
}

GUI_RemoteControlPreferences::GUI_RemoteControlPreferences(const QString& identifier) :
	Base(identifier) {}

GUI_RemoteControlPreferences::~GUI_RemoteControlPreferences() = default;

void GUI_RemoteControlPreferences::initUi()
{
	ui = std::make_shared<Ui::GUI_RemoteControlPreferences>();
	ui->setupUi(this);

	connect(ui->cbActivate, &QCheckBox::toggled, this, &GUI_RemoteControlPreferences::activeToggled);
	connect(ui->sbPort, spinbox_value_changed_int, this, &GUI_RemoteControlPreferences::portChanged);
	connect(ui->sbDiscover, spinbox_value_changed_int, this, &GUI_RemoteControlPreferences::discoverPortChanged);
}

void GUI_RemoteControlPreferences::retranslate()
{
	ui->retranslateUi(this);
	ui->labActive->setText(Lang::get(Lang::Active));

	const auto tooltip = tr("If activated, Sayonara will answer an UDP request that it is remote controllable");
	ui->cbDiscover->setToolTip(tooltip);
	ui->labDiscover->setToolTip(tooltip);
}

bool GUI_RemoteControlPreferences::commit()
{
	SetSetting(Set::Remote_Active, ui->cbActivate->isChecked());
	SetSetting(Set::Remote_Port, ui->sbPort->value());
	SetSetting(Set::Remote_Discoverable, ui->cbDiscover->isChecked());
	SetSetting(Set::Remote_DiscoverPort, ui->sbDiscover->value());

	return true;
}

void GUI_RemoteControlPreferences::revert()
{
	const auto active = GetSetting(Set::Remote_Active);
	ui->cbActivate->setChecked(active);

	ui->sbPort->setEnabled(active);
	ui->sbPort->setValue(GetSetting(Set::Remote_Port));

	ui->cbDiscover->setEnabled(active);
	ui->cbDiscover->setChecked(GetSetting(Set::Remote_Discoverable));

	ui->sbDiscover->setEnabled(active);
	ui->sbDiscover->setValue(GetSetting(Set::Remote_DiscoverPort));

	refreshUrl();
}

QString GUI_RemoteControlPreferences::actionName() const { return tr("Remote control"); }

void GUI_RemoteControlPreferences::activeToggled(const bool /*b*/)
{
	refreshUrl();

	const auto active = GetSetting(Set::Remote_Active);

	ui->sbPort->setEnabled(active);
	ui->cbDiscover->setEnabled(active);
	ui->sbDiscover->setEnabled(active);
}

void GUI_RemoteControlPreferences::portChanged(const int port)
{
	ui->sbDiscover->setValue(port + 1);
	refreshUrl();
}

void GUI_RemoteControlPreferences::discoverPortChanged(const int port)
{
	if(port == ui->sbPort->value())
	{
		ui->sbDiscover->setValue(ui->sbPort->value() + 1);
	}
}

void GUI_RemoteControlPreferences::refreshUrl()
{
	bool active = ui->cbActivate->isChecked();

	ui->labUrl->setVisible(active);
	ui->labUrls->setText(getUrlString(ui->sbPort->value()));
}

bool GUI_RemoteControlPreferences::hasError() const
{
	const auto remotePort = GetSetting(Set::Remote_Port);
	return (ui->sbPort->value() == remotePort) ||
	       (ui->sbDiscover->value() == remotePort);
}

QString GUI_RemoteControlPreferences::errorString() const
{
	const auto remotePort = GetSetting(Set::Remote_Port);
	return tr("Port %1 already in use").arg(remotePort);
}
