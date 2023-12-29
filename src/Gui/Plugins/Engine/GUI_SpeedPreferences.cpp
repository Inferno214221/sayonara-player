/* GUI_SpeedPreferences.cpp */

/* Copyright (C) 2011-2023 Michael Lugmair
 *
 * This file is part of Sayonara Player
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

#include "GUI_SpeedPreferences.h"
#include "Utils/Settings/Settings.h"
#include "Gui/Plugins/ui_GUI_SpeedPreferences.h"

GUI_SpeedPreferences::GUI_SpeedPreferences(QWidget* parent) :
	Dialog(parent),
	ui {std::make_shared<Ui::GUI_SpeedPreferences>()}
{
	ui->setupUi(this);

	connect(this, &QDialog::accepted, this, [this]() {
		SetSetting(Set::Speed_ShowSteps, ui->cbShowSpeedStepButtons->isChecked());
		SetSetting(Set::Speed_Step, ui->sbSpeedStepValue->value());
		SetSetting(Set::Speed_MinValue, ui->sbSpeedMinValue->value());
		SetSetting(Set::Speed_MaxValue, ui->sbSpeedMaxValue->value());
	});

	connect(ui->cbShowSpeedStepButtons, &QCheckBox::toggled, this, [this](const auto active) {
		ui->sbSpeedStepValue->setVisible(active);
		ui->labSpeedStepValue->setVisible(active);
	});
}

void GUI_SpeedPreferences::showEvent(QShowEvent* e)
{
	WidgetTemplate::showEvent(e);

	const auto showSpeedStepSettings = GetSetting(Set::Speed_ShowSteps);
	ui->cbShowSpeedStepButtons->setChecked(showSpeedStepSettings);
	ui->sbSpeedStepValue->setVisible(showSpeedStepSettings);
	ui->labSpeedStepValue->setVisible(showSpeedStepSettings);
	ui->sbSpeedStepValue->setValue(GetSetting(Set::Speed_Step));

	ui->sbSpeedMinValue->setValue(GetSetting(Set::Speed_MinValue));
	ui->sbSpeedMaxValue->setValue(GetSetting(Set::Speed_MaxValue));
}

GUI_SpeedPreferences::~GUI_SpeedPreferences() = default;
