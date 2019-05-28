/* GUI_UiPreferences.cpp */

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




#include "GUI_UiPreferences.h"
#include "GUI_FontConfig.h"
#include "GUI_IconPreferences.h"
#include "Gui/Utils/Style.h"
#include "Gui/Preferences/ui_GUI_UiPreferences.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"


struct GUI_UiPreferences::Private
{
	GUI_FontConfig* font_config=nullptr;
	GUI_IconPreferences* icon_config=nullptr;
};

GUI_UiPreferences::GUI_UiPreferences(const QString& identifier) :
	Preferences::Base(identifier)
{
	m = Pimpl::make<Private>();
}

GUI_UiPreferences::~GUI_UiPreferences() {}

QString GUI_UiPreferences::action_name() const
{
	return tr("User Interface");
}

bool GUI_UiPreferences::commit()
{
	m->font_config->commit();
	m->icon_config->commit();

	SetSetting(Set::Player_ControlStyle, ui->cb_big_cover->isChecked() ? 1 : 0);
	SetSetting(Set::Player_Style, ui->cb_dark_mode->isChecked() ? 1 : 0);

	return true;
}

void GUI_UiPreferences::revert()
{
	m->font_config->revert();
	m->icon_config->revert();

	style_changed();
}

void GUI_UiPreferences::init_ui()
{
	if(is_ui_initialized()){
		return;
	}

	setup_parent(this, &ui);

	m->font_config = new GUI_FontConfig(ui->tabWidget);
	m->icon_config = new GUI_IconPreferences(ui->tabWidget);

	ui->tabWidget->addTab(m->font_config, m->font_config->action_name());
	ui->tabWidget->addTab(m->icon_config, m->icon_config->action_name());

	ListenSetting(Set::Player_ControlStyle, GUI_UiPreferences::style_changed);
	ListenSetting(Set::Player_Style, GUI_UiPreferences::style_changed);

	retranslate_ui();
	revert();
}

void GUI_UiPreferences::style_changed()
{
	ui->cb_big_cover->setChecked(GetSetting(Set::Player_ControlStyle) == 1);
	ui->cb_dark_mode->setChecked(Style::is_dark());
}

void GUI_UiPreferences::retranslate_ui()
{
	ui->tabWidget->setTabText(0, tr("General"));
	ui->cb_big_cover->setText(tr("Show large cover"));
	ui->cb_dark_mode->setText(Lang::get(Lang::DarkMode));

	if(m->font_config){
		ui->tabWidget->setTabText(1, m->font_config->action_name());
	}

	if(m->icon_config){
		ui->tabWidget->setTabText(2, m->icon_config->action_name());
	}

	ui->retranslateUi(this);
}
