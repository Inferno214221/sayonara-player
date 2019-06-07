/* GUI_LastFmPreferences.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
 *      Author: Lucio Carreras
 */

#include "GUI_LastFmPreferences.h"
#include "Gui/Preferences/ui_GUI_LastFmPreferences.h"
#include "Utils/Crypt.h"

#include "Components/Streaming/LastFM/LastFM.h"

#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

struct GUI_LastFmPreferences::Private
{
	LastFM::Base* lfm=nullptr;

	Private(LastFM::Base* lfm) :
		lfm(lfm)
	{}

	~Private()
	{
		delete lfm; lfm = nullptr;
	}
};

GUI_LastFmPreferences::GUI_LastFmPreferences(const QString& identifier, LastFM::Base* lfm) :
	Base(identifier)
{
	m = Pimpl::make<Private>(lfm);
}

GUI_LastFmPreferences::~GUI_LastFmPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}


void GUI_LastFmPreferences::init_ui()
{
	setup_parent(this, &ui);

	revert();

	logged_in(m->lfm->is_logged_in());

	connect(ui->btn_login, &QPushButton::clicked, this, &GUI_LastFmPreferences::btn_login_clicked);
	connect(ui->cb_activate, &QCheckBox::toggled, this, &GUI_LastFmPreferences::active_changed);
	connect(m->lfm, &LastFM::Base::sig_logged_in, this, &GUI_LastFmPreferences::logged_in);
}


QString GUI_LastFmPreferences::action_name() const
{
	return "Last.fm";
}

void GUI_LastFmPreferences::retranslate_ui()
{
	ui->retranslateUi(this);
	ui->lab_activate->setText(Lang::get(Lang::Active));
	ui->lab_sec->setText(Lang::get(Lang::Seconds));

	logged_in(m->lfm->is_logged_in());
}

bool GUI_LastFmPreferences::commit()
{
	bool active = ui->cb_activate->isChecked();
	QString username = ui->tf_username->text();
	QString password = ui->tf_password->text();

	SetSetting(Set::LFM_Username, username);
	SetSetting(Set::LFM_Password, Util::Crypt::encrypt(password));
	SetSetting(Set::LFM_ScrobbleTimeSec, ui->sb_scrobble_time->value());
	SetSetting(Set::LFM_Active, active);

	return true;
}


void GUI_LastFmPreferences::revert()
{
	bool active = GetSetting(Set::LFM_Active);
	QString username = GetSetting(Set::LFM_Username);
	QString password = Util::Crypt::decrypt(GetSetting(Set::LFM_Password));

	active_changed(active);
	logged_in(m->lfm->is_logged_in());

	ui->tf_username->setText(username);
	ui->tf_password->setText(password);
	ui->sb_scrobble_time->setValue( GetSetting(Set::LFM_ScrobbleTimeSec) );
	ui->cb_activate->setChecked(active);
}


void GUI_LastFmPreferences::btn_login_clicked()
{
	if(ui->tf_username->text().length() < 3) {
		return;
	}

	if(ui->tf_password->text().length() < 3) {
		return;
	}

	ui->btn_login->setEnabled(false);

	QString username = ui->tf_username->text();
	QString password = ui->tf_password->text();

	m->lfm->login(username, password);
}


void GUI_LastFmPreferences::active_changed(bool active)
{
	if(!is_ui_initialized()){
		return;
	}

	ui->tf_username->setEnabled(active);
	ui->tf_password->setEnabled(active);
}


void GUI_LastFmPreferences::logged_in(bool success)
{
	if(!is_ui_initialized()){
		return;
	}

	if(success){
		ui->lab_status->setText(tr("Logged in"));
	}

	else{
		ui->lab_status->setText(tr("Not logged in"));
	}

	ui->btn_login->setEnabled(true);
}

