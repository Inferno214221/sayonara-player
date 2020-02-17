/* GUI_UiPreferences.cpp */

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

#include "GUI_UiPreferences.h"
#include "GUI_FontPreferences.h"
#include "GUI_IconPreferences.h"
#include "GUI_CssEditor.h"
#include "Gui/Utils/Style.h"
#include "Gui/Preferences/ui_GUI_UiPreferences.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"


struct GUI_UiPreferences::Private
{
	GUI_FontPreferences* fontConfig=nullptr;
	GUI_IconPreferences* iconConfig=nullptr;
};

GUI_UiPreferences::GUI_UiPreferences(const QString& identifier) :
	Preferences::Base(identifier)
{
	m = Pimpl::make<Private>();
}

GUI_UiPreferences::~GUI_UiPreferences() = default;

QString GUI_UiPreferences::actionName() const
{
	return tr("User Interface");
}

bool GUI_UiPreferences::commit()
{
	m->fontConfig->commit();
	m->iconConfig->commit();

	SetSetting(Set::Player_ControlStyle, ui->cb_bigCover->isChecked() ? 1 : 0);
	SetSetting(Set::Player_Style, ui->cb_darkMode->isChecked() ? 1 : 0);
	SetSetting(Set::Player_FadingCover, ui->cb_fadingCover->isChecked());

	Set::shout<SetNoDB::Player_MetaStyle>();

	return true;
}

void GUI_UiPreferences::revert()
{
	m->fontConfig->revert();
	m->iconConfig->revert();

	ui->cb_fadingCover->setChecked(GetSetting(Set::Player_FadingCover));

	styleChanged();
}

void GUI_UiPreferences::initUi()
{
	if(isUiInitialized()){
		return;
	}

	setupParent(this, &ui);

	m->fontConfig = new GUI_FontPreferences(ui->tabWidget);
	m->iconConfig = new GUI_IconPreferences(ui->tabWidget);

	ui->tabWidget->addTab(m->fontConfig, m->fontConfig->actionName());
	ui->tabWidget->addTab(m->iconConfig, m->iconConfig->actionName());

	connect(ui->btn_editCss, &QPushButton::clicked, this, &GUI_UiPreferences::editCssClicked);

	ListenSetting(Set::Player_ControlStyle, GUI_UiPreferences::styleChanged);
	ListenSetting(Set::Player_Style, GUI_UiPreferences::styleChanged);

	retranslate();
	revert();
}

void GUI_UiPreferences::styleChanged()
{
	ui->cb_bigCover->setChecked(GetSetting(Set::Player_ControlStyle) == 1);
	ui->cb_darkMode->setChecked(Style::isDark());
}

void GUI_UiPreferences::editCssClicked()
{
	GUI_CssEditor* editor = new GUI_CssEditor(this);
	editor->show();
}

void GUI_UiPreferences::retranslate()
{
	ui->tabWidget->setTabText(0, tr("General"));
	ui->cb_bigCover->setText(tr("Show large cover"));
	ui->cb_darkMode->setText(Lang::get(Lang::DarkMode));

	if(m->fontConfig){
		ui->tabWidget->setTabText(1, m->fontConfig->actionName());
	}

	if(m->iconConfig){
		ui->tabWidget->setTabText(2, m->iconConfig->actionName());
	}

	ui->retranslateUi(this);
}
