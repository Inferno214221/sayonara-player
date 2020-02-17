/* GUI_StreamPreferences.cpp */

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



#include "GUI_StreamPreferences.h"
#include "ui_GUI_StreamPreferences.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"


GUI_StreamPreferences::GUI_StreamPreferences(const QString& identifier) :
	Preferences::Base(identifier)
{}

GUI_StreamPreferences::~GUI_StreamPreferences() {}

bool GUI_StreamPreferences::commit()
{
	SetSetting(Set::Stream_NewTab, ui->cbNewTab->isChecked());
	SetSetting(Set::Stream_ShowHistory, ui->cbShowHistory->isChecked());
	SetSetting(Set::Engine_BufferSizeMS, ui->sbBufferSize->value());

	return true;
}

void GUI_StreamPreferences::revert()
{
	ui->cbShowHistory->setChecked(GetSetting(Set::Stream_ShowHistory));
	ui->cbNewTab->setChecked(GetSetting(Set::Stream_NewTab));
	ui->sbBufferSize->setValue(GetSetting(Set::Engine_BufferSizeMS));
}

QString GUI_StreamPreferences::actionName() const
{
	QString s = Lang::get(Lang::Streams);
	QString p = Lang::get(Lang::Podcasts);

	return tr("%1 and %2").arg(s).arg(p);
}

void GUI_StreamPreferences::initUi()
{
	if(isUiInitialized()){
		return;
	}

	setupParent(this, &ui);

	revert();
}

void GUI_StreamPreferences::retranslate()
{
	ui->retranslateUi(this);
}
