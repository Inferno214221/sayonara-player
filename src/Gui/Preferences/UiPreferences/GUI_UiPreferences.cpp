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
#include "GUI_IconPreferences.h"
#include "GUI_CssEditor.h"
#include "Gui/Utils/Style.h"
#include "Gui/Preferences/ui_GUI_UiPreferences.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Macros.h"

struct GUI_UiPreferences::Private
{
	GUI_IconPreferences* iconConfig = nullptr;
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
	m->iconConfig->commit();

	SetSetting(Set::Player_ControlStyle, ui->cbBigCover->isChecked() ? 1 : 0);
	SetSetting(Set::Player_Style, ui->cbDarkMode->isChecked() ? 1 : 0);
	SetSetting(Set::Player_FadingCover, ui->cbFadingCover->isChecked());
	SetSetting(Set::Lib_FontBold, ui->cbBoldLibraryFont->isChecked());
#ifdef DISABLE_NATIVE_DIR_DIALOGS
	SetSetting(Set::Player_ForceNativeDirDialog, ui->cbForceNativeDirDialog->isChecked());
#endif

	Set::shout<Set::Player_Style>();

	return true;
}

void GUI_UiPreferences::revert()
{
	m->iconConfig->revert();

	ui->cbFadingCover->setChecked(GetSetting(Set::Player_FadingCover));
	ui->cbBoldLibraryFont->setChecked(GetSetting(Set::Lib_FontBold));
#ifdef DISABLE_NATIVE_DIR_DIALOGS
	ui->cbForceNativeDirDialog->setChecked(GetSetting(Set::Player_ForceNativeDirDialog));
#endif

	styleChanged();
}

void GUI_UiPreferences::initUi()
{
	if(isUiInitialized())
	{
		return;
	}

	setupParent(this, &ui);

#ifdef DISABLE_NATIVE_DIR_DIALOGS
	ui->cbForceNativeDirDialog->setVisible(true);
#else
	ui->cbForceNativeDirDialog->setVisible(false);
#endif

	m->iconConfig = new GUI_IconPreferences(ui->tabWidget);

	ui->tabWidget->addTab(m->iconConfig, m->iconConfig->actionName());

	connect(ui->btn_editCss, &QPushButton::clicked, this, &GUI_UiPreferences::editCssClicked);

	ListenSetting(Set::Player_ControlStyle, GUI_UiPreferences::styleChanged);
	ListenSetting(Set::Player_Style, GUI_UiPreferences::styleChanged);

	retranslate();
	revert();
}

void GUI_UiPreferences::styleChanged()
{
	ui->cbBigCover->setChecked(GetSetting(Set::Player_ControlStyle) == 1);
	ui->cbDarkMode->setChecked(Style::isDark());
}

void GUI_UiPreferences::editCssClicked()
{
	GUI_CssEditor* editor = new GUI_CssEditor(this);
	editor->show();
}

void GUI_UiPreferences::retranslate()
{
	ui->tabWidget->setTabText(0, tr("General"));
	ui->cbBigCover->setText(tr("Show large cover"));
	ui->cbDarkMode->setText(Lang::get(Lang::DarkMode));

	if(m->iconConfig)
	{
		ui->tabWidget->setTabText(2, m->iconConfig->actionName());
	}

	ui->retranslateUi(this);
}
