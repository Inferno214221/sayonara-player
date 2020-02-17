/* GUI_PlaylistPreferences.cpp */

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


/* GUI_PlaylistPreferences.cpp */

#include "GUI_PlaylistPreferences.h"
#include "Gui/Preferences/ui_GUI_PlaylistPreferences.h"
#include "Gui/Utils/Icons.h"

#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

namespace Algorithm=Util::Algorithm;

static
bool evaluate_expression(const QString& expr)
{
	if(expr.isEmpty()){
		return false;
	}

	int star_count=0;
	int apostroph_count=0;
	int percent_count=0;
	for(QChar c : expr)
	{
		if(c == '\''){
			apostroph_count ++;
		}

		else if(c == '*'){
			star_count ++;
		}

		else if(c == '%'){
			percent_count ++;
		}
	}

	if(apostroph_count % 2 == 1){
		return false;
	}

	if(percent_count % 2 == 1){
		return false;
	}

	if(star_count % 2 == 1){
		return false;
	}

	QStringList between_percents;
	QRegExp re("%(.*)%");
	re.setMinimal(true);

	int idx = re.indexIn(expr);
	while(idx >= 0 && idx < expr.size()){
		QString cap = re.cap(1);
		between_percents << cap;
		idx = re.indexIn(expr, idx + 1 + cap.size());
	}

	int correct_ones = 0;
	int incorrect_ones = 0;

	for(const QString& between_percent : Algorithm::AsConst(between_percents))
	{
		if((between_percent.compare("nr") != 0) &&
		   (between_percent.compare("title") != 0) &&
		   (between_percent.compare("artist") != 0) &&
		   (between_percent.compare("album") != 0))
		{
			incorrect_ones++;
		}

		else {
			correct_ones++;
		}
	}

	return (correct_ones > incorrect_ones);
}


GUI_PlaylistPreferences::GUI_PlaylistPreferences(const QString& identifier) :
	Base(identifier) {}

GUI_PlaylistPreferences::~GUI_PlaylistPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}


bool GUI_PlaylistPreferences::commit()
{
	SetSetting(Set::PL_LoadSavedPlaylists, ui->cbLoadSavedPlaylists->isChecked() );
	SetSetting(Set::PL_LoadTemporaryPlaylists, ui->cbLoadTemporaryPlaylists->isChecked() );
	SetSetting(Set::PL_LoadLastTrack, (ui->cbLoadLastTrack->isChecked() && ui->cbLoadLastTrack->isEnabled()) );
	SetSetting(Set::PL_RememberTime, (ui->cbRememberTime->isChecked() && ui->cbRememberTime->isEnabled()) );
	SetSetting(Set::PL_StartPlaying, (ui->cbStartPlaying->isChecked() && ui->cbStartPlaying->isEnabled()) );

	SetSetting(Set::PL_ShowNumbers, ui->cbShowNumbers->isChecked());
	SetSetting(Set::PL_ShowCovers, ui->cbShowCovers->isChecked());
	SetSetting(Set::PL_ShowRating, ui->cbShowRating->isChecked());

	SetSetting(Set::PL_ShowClearButton, ui->cbShowClearButton->isChecked());
	SetSetting(Set::PL_RememberTrackAfterStop, ui->cbRememberAfterStop->isChecked());

	if(evaluate_expression(ui->leExpression->text())){
		SetSetting(Set::PL_EntryLook, ui->leExpression->text());
		return true;
	}

	return false;
}

void GUI_PlaylistPreferences::revert()
{
	bool load_saved_playlists, load_temporary_playlists, load_last_track, remember_time, start_playing;

	load_saved_playlists = GetSetting(Set::PL_LoadSavedPlaylists);
	load_temporary_playlists = GetSetting(Set::PL_LoadTemporaryPlaylists);
	load_last_track = GetSetting(Set::PL_LoadLastTrack);
	remember_time = GetSetting(Set::PL_RememberTime);
	start_playing = GetSetting(Set::PL_StartPlaying);

	ui->cbLoadSavedPlaylists->setChecked(load_saved_playlists);
	ui->cbLoadTemporaryPlaylists->setChecked(load_temporary_playlists);
	ui->cbLoadLastTrack->setChecked(load_last_track);
	ui->cbRememberTime->setChecked(remember_time);
	ui->cbStartPlaying->setChecked(start_playing);

	ui->leExpression->setText(GetSetting(Set::PL_EntryLook));
	ui->cbShowNumbers->setChecked(GetSetting(Set::PL_ShowNumbers));
	ui->cbShowCovers->setChecked(GetSetting(Set::PL_ShowCovers));
	ui->cbShowRating->setChecked(GetSetting(Set::PL_ShowRating));
	ui->cbShowClearButton->setChecked(GetSetting(Set::PL_ShowClearButton));
	ui->cbRememberAfterStop->setChecked(GetSetting(Set::PL_RememberTrackAfterStop));
}


void GUI_PlaylistPreferences::initUi()
{
	if(isUiInitialized()){
		return;
	}

	setupParent(this, &ui);
	ui->tabWidget->setCurrentIndex(0);
	ui->leExpression->setStyleSheet("font-family: mono;");

	revert();

	checkboxToggled(true);

	connect(ui->cbLoadLastTrack, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::checkboxToggled);
	connect(ui->cbLoadSavedPlaylists, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::checkboxToggled);
	connect(ui->cbLoadTemporaryPlaylists, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::checkboxToggled);
	connect(ui->cbRememberTime, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::checkboxToggled);
	connect(ui->cbStartPlaying, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::checkboxToggled);

	connect(ui->btnDefault, &QPushButton::clicked, this, [=]()
	{
		ui->leExpression->setText("*%title%* - %artist%");
	});
}

QString GUI_PlaylistPreferences::actionName() const
{
	return Lang::get(Lang::Playlist);
}

void GUI_PlaylistPreferences::retranslate()
{
	ui->retranslateUi(this);

	ui->labAlbum->setText(Lang::get(Lang::Album));
	ui->labArtist->setText(Lang::get(Lang::Artist));
	ui->labTitle->setText(Lang::get(Lang::Title));
	ui->labTrackNumber->setText(Lang::get(Lang::TrackNo));
	ui->btnDefault->setText(Lang::get(Lang::Default));
}

void GUI_PlaylistPreferences::skinChanged()
{
	if(ui)
	{
		ui->btnDefault->setIcon(Gui::Icons::icon(Gui::Icons::Undo));
	}
}

QString GUI_PlaylistPreferences::errorString() const
{
	return tr("Playlist look: Invalid expression");
}

void GUI_PlaylistPreferences::checkboxToggled(bool b)
{
	Q_UNUSED(b);

	bool load = (ui->cbLoadSavedPlaylists->isChecked() || ui->cbLoadTemporaryPlaylists->isChecked());

	ui->cbLoadLastTrack->setEnabled(load);
	ui->cbRememberTime->setEnabled(load);
	ui->cbStartPlaying->setEnabled(load);

	bool cbLoadLastTrack_checked = ui->cbLoadLastTrack->isChecked() && ui->cbLoadLastTrack->isEnabled();
	ui->cbRememberTime->setEnabled(cbLoadLastTrack_checked);
}
