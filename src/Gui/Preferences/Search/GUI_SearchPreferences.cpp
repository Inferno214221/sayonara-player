/* GUI_SearchPreferences.cpp */

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



#include "GUI_SearchPreferences.h"
#include "Gui/ui_GUI_SearchPreferences.h"

#include "Utils/Language/Language.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Settings/Settings.h"

GUI_SearchPreferences::GUI_SearchPreferences(const QString& identifier) :
	Preferences::Base (identifier)
{

}

GUI_SearchPreferences::~GUI_SearchPreferences()
{
	if(ui) {
		delete ui; ui=nullptr;
	}
}

QString GUI_SearchPreferences::action_name() const
{
	return Lang::get(Lang::SearchNoun);
}

bool GUI_SearchPreferences::commit()
{
	Library::SearchModeMask mask = 0;

	if(ui->cb_case_insensitive->isChecked()){
		mask |= Library::CaseInsensitve;
	}

	if(ui->cb_no_special_chars->isChecked()){
		mask |= Library::NoSpecialChars;
	}

	if(ui->cb_no_accents->isChecked()){
		mask |= Library::NoDiacriticChars;
	}

	SetSetting(Set::Lib_SearchMode, mask);

	return true;
}

void GUI_SearchPreferences::revert()
{
	Library::SearchModeMask mask = GetSetting(Set::Lib_SearchMode);

	ui->cb_case_insensitive->setChecked(mask & Library::CaseInsensitve);
	ui->cb_no_special_chars->setChecked(mask & Library::NoSpecialChars);
	ui->cb_no_accents->setChecked(mask & Library::NoDiacriticChars);
}

void GUI_SearchPreferences::init_ui()
{
	setup_parent(this, &ui);

	revert();
}

void GUI_SearchPreferences::retranslate_ui()
{
	ui->retranslateUi(this);
}
