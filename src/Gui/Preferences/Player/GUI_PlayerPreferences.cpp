/* GUI_PlayerPreferences.cpp */

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

void GUI_PlayerPreferences::init_ui()
{
	setup_parent(this, &ui);

	ui->cb_logger->addItem(Lang::get(Lang::Default));
	ui->cb_logger->addItem("Debug");
	ui->cb_logger->addItem("Develop");
	ui->cb_logger->addItem("Crazy");

	revert();

	connect(ui->cb_show_tray_icon, &QCheckBox::toggled, this, &GUI_PlayerPreferences::show_tray_icon_toggled);
	connect(ui->cb_start_in_tray, &QCheckBox::toggled, this, &GUI_PlayerPreferences::show_tray_icon_toggled);
	connect(ui->cb_close_to_tray, &QCheckBox::toggled, this, &GUI_PlayerPreferences::show_tray_icon_toggled);
}


QString GUI_PlayerPreferences::action_name() const
{
	return Lang::get(Lang::Application);
}


bool GUI_PlayerPreferences::commit()
{
	SetSetting(Set::Player_Min2Tray, ui->cb_close_to_tray->isChecked());
	SetSetting(Set::Player_StartInTray, ui->cb_start_in_tray->isChecked());
	SetSetting(Set::Player_ShowTrayIcon, ui->cb_show_tray_icon->isChecked());
	SetSetting(Set::Player_NotifyNewVersion, ui->cb_update_notifications->isChecked());
	SetSetting(Set::Logger_Level, ui->cb_logger->currentIndex());

	return true;
}

void GUI_PlayerPreferences::revert()
{
	bool show_tray_icon = GetSetting(Set::Player_ShowTrayIcon);

	ui->cb_start_in_tray->setChecked(GetSetting(Set::Player_StartInTray));
	ui->cb_close_to_tray->setChecked(GetSetting(Set::Player_Min2Tray));
	ui->cb_update_notifications->setChecked(GetSetting(Set::Player_NotifyNewVersion));
	ui->cb_show_tray_icon->setChecked(GetSetting(Set::Player_ShowTrayIcon));
	ui->cb_logger->setCurrentIndex(GetSetting(Set::Logger_Level));

	show_tray_icon_toggled(show_tray_icon);
}

void GUI_PlayerPreferences::show_tray_icon_toggled(bool b)
{
	Q_UNUSED(b)

	bool show_warning =
	(
		(!ui->cb_show_tray_icon->isChecked()) &&
		(ui->cb_start_in_tray->isChecked() || ui->cb_close_to_tray->isChecked())
	);

	ui->lab_warning_header->setVisible(show_warning);
	ui->lab_warning->setVisible(show_warning);
}

void GUI_PlayerPreferences::retranslate_ui()
{
	ui->retranslateUi(this);

	ui->lab_logger->setText(Lang::get(Lang::Logger));
	ui->cb_logger->setItemText(0, Lang::get(Lang::Default));

	QString text =
		tr("This might cause Sayonara not to show up again.") + " " +
		tr("In this case use the '--show' option at the next startup.");

	ui->lab_warning_header->setText(Lang::get(Lang::Warning));
	ui->lab_warning->setText(text);
}
