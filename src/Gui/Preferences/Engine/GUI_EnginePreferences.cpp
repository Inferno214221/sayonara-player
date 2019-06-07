/* GUI_EnginePreferences.cpp */

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



#include "GUI_EnginePreferences.h"
#include "Gui/Preferences/ui_GUI_EnginePreferences.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"

GUI_EnginePreferences::GUI_EnginePreferences(const QString& identifier) :
	Preferences::Base(identifier)
{}


GUI_EnginePreferences::~GUI_EnginePreferences()
{
	if(ui){
		delete ui; ui = nullptr;
	}
}


QString GUI_EnginePreferences::action_name() const
{
	return tr("Audio");
}

bool GUI_EnginePreferences::commit()
{
	if(ui->rb_pulse->isChecked()){
		SetSetting(Set::Engine_Sink, QString("pulse"));
	}

	else if(ui->rb_alsa->isChecked()){
		SetSetting(Set::Engine_Sink, QString("alsa"));
	}

	else{
		SetSetting(Set::Engine_Sink, QString("auto"));
	}

	return true;
}

void GUI_EnginePreferences::revert()
{
	QString engine_name = GetSetting(Set::Engine_Sink);
	if(engine_name == "pulse"){
		ui->rb_pulse->setChecked(true);
	}

	else if(engine_name == "alsa"){
		ui->rb_alsa->setChecked(true);
	}

	else{
		ui->rb_auto->setChecked(true);
	}
}

void GUI_EnginePreferences::init_ui()
{
	if(ui){
		return;
	}

	setup_parent(this, &ui);

	revert();
}

void GUI_EnginePreferences::retranslate_ui()
{
	ui->retranslateUi(this);

	ui->rb_auto->setText(Lang::get(Lang::Automatic));
}

