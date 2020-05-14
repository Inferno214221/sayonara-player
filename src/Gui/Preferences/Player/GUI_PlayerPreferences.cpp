/* GUI_PlayerPreferences.cpp */

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

#include "GUI_PlayerPreferences.h"
#include "Gui/Preferences/ui_GUI_PlayerPreferences.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"


GUI_PlayerPreferences::GUI_PlayerPreferences(const QString& identifier) :
	Base(identifier) {}

GUI_PlayerPreferences::~GUI_PlayerPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_PlayerPreferences::initUi()
{
	setupParent(this, &ui);

	ui->widgetWarning->setVisible(false);

	ui->cbLogger->addItem(Lang::get(Lang::Default));
	ui->cbLogger->addItem("Debug");
	ui->cbLogger->addItem("Develop");
	ui->cbLogger->addItem("Crazy");

	revert();

	connect(ui->cbShowTrayIcon, &QCheckBox::toggled, this, &GUI_PlayerPreferences::showTrayIconToggled);
	connect(ui->cbStartInTray, &QCheckBox::toggled, this, &GUI_PlayerPreferences::showTrayIconToggled);
	connect(ui->cbCloseToTray, &QCheckBox::toggled, this, &GUI_PlayerPreferences::showTrayIconToggled);
}


QString GUI_PlayerPreferences::actionName() const
{
	return Lang::get(Lang::Application);
}


bool GUI_PlayerPreferences::commit()
{
	SetSetting(Set::Player_Min2Tray, ui->cbCloseToTray->isChecked());
	SetSetting(Set::Player_StartInTray, ui->cbStartInTray->isChecked());
	SetSetting(Set::Player_ShowTrayIcon, ui->cbShowTrayIcon->isChecked());
	SetSetting(Set::Player_NotifyNewVersion, ui->cbUpdateNotifications->isChecked());
	SetSetting(Set::Logger_Level, ui->cbLogger->currentIndex());

	return true;
}

void GUI_PlayerPreferences::revert()
{
	bool showTrayIcon = GetSetting(Set::Player_ShowTrayIcon);

	ui->cbStartInTray->setChecked(GetSetting(Set::Player_StartInTray));
	ui->cbCloseToTray->setChecked(GetSetting(Set::Player_Min2Tray));
	ui->cbUpdateNotifications->setChecked(GetSetting(Set::Player_NotifyNewVersion));
	ui->cbShowTrayIcon->setChecked(GetSetting(Set::Player_ShowTrayIcon));
	ui->cbLogger->setCurrentIndex(GetSetting(Set::Logger_Level));

	showTrayIconToggled(showTrayIcon);
}

void GUI_PlayerPreferences::showTrayIconToggled(bool b)
{
	Q_UNUSED(b)

	bool showWarning =
	(
		(!ui->cbShowTrayIcon->isChecked()) &&
		(ui->cbStartInTray->isChecked() || ui->cbCloseToTray->isChecked())
	);

	ui->widgetWarning->setVisible(showWarning);
}

void GUI_PlayerPreferences::retranslate()
{
	ui->retranslateUi(this);

	ui->labLogger->setText(Lang::get(Lang::Logger));
	ui->cbLogger->setItemText(0, Lang::get(Lang::Default));

	QString text =
		tr("This might cause Sayonara not to show up again.") + " " +
		tr("In this case use the '--show' option at the next startup.");

	ui->labWarningHeader->setText(Lang::get(Lang::Warning));
	ui->labWarning->setText(text);
}
