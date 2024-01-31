/* GUI_LastFmPreferences.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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


/*
 * GUI_LastFmPreferences.cpp
 *
 *  Created on: Apr 21, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "GUI_LastFmPreferences.h"
#include "Gui/Preferences/ui_GUI_LastFmPreferences.h"
#include "Gui/Utils/Style.h"

#include "Components/Streaming/LastFM/LastFM.h"

#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/Crypt.h"

struct GUI_LastFmPreferences::Private
{
	LastFM::Base* lastFm;

	explicit Private(LastFM::Base* lastFm) :
		lastFm {lastFm} {}
};

GUI_LastFmPreferences::GUI_LastFmPreferences(const QString& identifier, LastFM::Base* lastFm) :
	Base(identifier),
	m {Pimpl::make<Private>(lastFm)} {}

GUI_LastFmPreferences::~GUI_LastFmPreferences() = default;

void GUI_LastFmPreferences::initUi()
{
	setupParent(this, &ui);

	revert();

	loginFinished(m->lastFm->isLoggedIn());

	connect(ui->btnLogin, &QPushButton::clicked, this, &GUI_LastFmPreferences::loginClicked);
	connect(ui->cbActivate, &QCheckBox::toggled, this, &GUI_LastFmPreferences::activeChanged);
	connect(m->lastFm, &LastFM::Base::sigLoggedIn, this, &GUI_LastFmPreferences::loginFinished);
}

QString GUI_LastFmPreferences::actionName() const { return "Last.fm"; }

void GUI_LastFmPreferences::retranslate()
{
	ui->retranslateUi(this);
	ui->labActivate->setText(Lang::get(Lang::Active));
	ui->labSeconds->setText(Lang::get(Lang::Seconds));

	loginFinished(m->lastFm->isLoggedIn());
}

bool GUI_LastFmPreferences::commit()
{
	const auto active = ui->cbActivate->isChecked();
	const auto username = ui->leUsername->text();
	const auto password = ui->lePassword->text();

	SetSetting(Set::LFM_Username, username);
	SetSetting(Set::LFM_Password, Util::Crypt::encrypt(password));
	SetSetting(Set::LFM_ScrobbleTimeSec, ui->sbScrobbleTime->value());
	SetSetting(Set::LFM_Active, active);

	return true;
}

void GUI_LastFmPreferences::revert()
{
	const auto active = GetSetting(Set::LFM_Active);
	const auto username = GetSetting(Set::LFM_Username);
	const auto password = Util::Crypt::decrypt(GetSetting(Set::LFM_Password));

	activeChanged(active);
	loginFinished(m->lastFm->isLoggedIn());

	ui->leUsername->setText(username);
	ui->lePassword->setText(password);
	ui->sbScrobbleTime->setValue(GetSetting(Set::LFM_ScrobbleTimeSec));
	ui->cbActivate->setChecked(active);

	const auto link = Util::createLink("https://last.fm", Style::isDark(), true);
	ui->labWebsite->setText(link);
}

void GUI_LastFmPreferences::loginClicked()
{
	if((ui->leUsername->text().length() < 3) ||
	   (ui->lePassword->text().length() < 3))
	{
		return;
	}

	ui->btnLogin->setEnabled(false);

	m->lastFm->login(ui->leUsername->text(), ui->lePassword->text());
}

void GUI_LastFmPreferences::activeChanged(const bool active)
{
	if(ui)
	{
		ui->leUsername->setEnabled(active);
		ui->lePassword->setEnabled(active);
	}
}

void GUI_LastFmPreferences::loginFinished(const bool success)
{
	if(ui)
	{
		const auto text = success ? tr("Logged in") : tr("Not logged in");
		ui->labStatus->setText(text);
		ui->btnLogin->setEnabled(true);
	}
}
