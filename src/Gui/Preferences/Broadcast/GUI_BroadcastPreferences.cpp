/* GUI_BroadcastSetup.cpp */

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

#include "GUI_BroadcastPreferences.h"
#include "Gui/Preferences/ui_GUI_BroadcastPreferences.h"

#include "Gui/Utils/Widgets/WidgetTemplate.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

namespace
{
	QString urlString(const int port)
	{
		const auto ips = Util::ipAddresses();

		QStringList ret;
		Util::Algorithm::transform(ips, ret, [&](const auto& ip) {
			return QString("http://%1:%2/playlist.m3u")
				.arg(ip)
				.arg(port);
		});

		return ret.join("; ");
	}
};

GUI_BroadcastPreferences::GUI_BroadcastPreferences(const QString& identifier) :
	Base(identifier) {}

GUI_BroadcastPreferences::~GUI_BroadcastPreferences() = default;

void GUI_BroadcastPreferences::initUi()
{
	ui = std::make_shared<Ui::GUI_BroadcastPreferences>();
	ui->setupUi(this);

	connect(ui->cbActive, &QCheckBox::toggled, this, [this](const auto /*b*/) { refreshUrl(); });
	connect(ui->sbPort, spinbox_value_changed_int, this, [this](const auto /*port*/) { refreshUrl(); });
}

bool GUI_BroadcastPreferences::commit()
{
	SetSetting(Set::Broadcast_Active, ui->cbActive->isChecked());
	SetSetting(Set::Broadcast_Prompt, ui->cbPrompt->isChecked());
	SetSetting(Set::Broadcast_Port, ui->sbPort->value());

	return true;
}

void GUI_BroadcastPreferences::revert()
{
	const auto active = GetSetting(Set::Broadcast_Active);

	ui->cbActive->setChecked(active);
	ui->cbPrompt->setChecked(GetSetting(Set::Broadcast_Prompt));
	ui->sbPort->setValue(GetSetting(Set::Broadcast_Port));
	ui->leUrl->setVisible(active);
	ui->labUrlTitle->setVisible(active);

	refreshUrl();
}

void GUI_BroadcastPreferences::retranslate()
{
	ui->retranslateUi(this);

	ui->labActive->setText(Lang::get(Lang::Active));
	ui->labUrlTitle->setText(Lang::get(Lang::StreamUrl));
}

QString GUI_BroadcastPreferences::actionName() const { return Lang::get(Lang::Broadcast); }

void GUI_BroadcastPreferences::refreshUrl()
{
	const auto active = ui->cbActive->isChecked();

	ui->leUrl->setVisible(active);
	ui->leUrl->setText(urlString(ui->sbPort->value()));
	ui->labUrlTitle->setVisible(active);
}

bool GUI_BroadcastPreferences::hasError() const
{
	const auto port = ui->sbPort->value();
	return (port == GetSetting(Set::Remote_Port)) ||
	       (port == GetSetting(Set::Remote_DiscoverPort));
}

QString GUI_BroadcastPreferences::errorString() const
{
	return tr("Port %1 already in use").arg(ui->sbPort->value());
}
