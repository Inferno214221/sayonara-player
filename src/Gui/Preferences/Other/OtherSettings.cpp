/* OtherSettings.cpp, (Created on 08.01.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
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

#include "OtherSettings.h"
#include "ui_GUI_OtherSettings.h"

#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

GUI_OtherSettings::GUI_OtherSettings(const QString& identifier) :
	Preferences::Base(identifier) {}

GUI_OtherSettings::~GUI_OtherSettings() = default;

QString GUI_OtherSettings::actionName() const { return tr("Other settings"); }

bool GUI_OtherSettings::commit()
{
	SetSetting(Set::Tagging_UseSelectiveTagging, ui->cbSelectiveTagUpdate->isChecked());
	return true;
}

void GUI_OtherSettings::revert()
{
	ui->cbSelectiveTagUpdate->setChecked(GetSetting(Set::Tagging_UseSelectiveTagging));
}

void GUI_OtherSettings::initUi()
{
	ui = std::make_shared<Ui::GUI_OtherSettings>();
	ui->setupUi(this);
}

void GUI_OtherSettings::retranslate()
{
	ui->retranslateUi(this);
}

bool GUI_OtherSettings::hasError() const { return false; }

QString GUI_OtherSettings::errorString() const { return {}; }
