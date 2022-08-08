/* GUI_SearchPreferences.cpp */

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

#include "GUI_SearchPreferences.h"
#include "Gui/Preferences/ui_GUI_SearchPreferences.h"

#include "Utils/globals.h"
#include "Utils/Language/Language.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Settings/Settings.h"

GUI_SearchPreferences::GUI_SearchPreferences(const QString& identifier) :
	Preferences::Base(identifier) {}

GUI_SearchPreferences::~GUI_SearchPreferences()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

QString GUI_SearchPreferences::actionName() const
{
	return Lang::get(Lang::SearchNoun);
}

bool GUI_SearchPreferences::commit()
{
	auto searchModeMask = Library::SearchModeMask {0};

	if(ui->cbCaseInsensitive->isChecked())
	{
		searchModeMask |= +Library::SearchMode::CaseInsensitve;
	}

	if(ui->cbNoSpecialChars->isChecked())
	{
		searchModeMask |= +Library::SearchMode::NoSpecialChars;
	}

	if(ui->cbNoAccents->isChecked())
	{
		searchModeMask |= +Library::SearchMode::NoDiacriticChars;
	}

	SetSetting(Set::Lib_SearchMode, searchModeMask);
	SetSetting(Set::Lib_SearchStringLength, ui->sbSearchStringLength->value());

	return true;
}

void GUI_SearchPreferences::revert()
{
	const auto searchModeMask = GetSetting(Set::Lib_SearchMode);

	ui->cbCaseInsensitive->setChecked(searchModeMask & +Library::SearchMode::CaseInsensitve);
	ui->cbNoSpecialChars->setChecked(searchModeMask & +Library::SearchMode::NoSpecialChars);
	ui->cbNoAccents->setChecked(searchModeMask & +Library::SearchMode::NoDiacriticChars);
	ui->sbSearchStringLength->setValue(GetSetting(Set::Lib_SearchStringLength));
}

void GUI_SearchPreferences::initUi()
{
	setupParent(this, &ui);

	revert();
}

void GUI_SearchPreferences::retranslate()
{
	ui->retranslateUi(this);

	ui->cbCaseInsensitive->setText(Lang::get(Lang::CaseInsensitive));
	ui->cbNoAccents->setText(Lang::get(Lang::IgnoreAccents));
	ui->cbNoSpecialChars->setText(Lang::get(Lang::IgnoreSpecialChars));
}
