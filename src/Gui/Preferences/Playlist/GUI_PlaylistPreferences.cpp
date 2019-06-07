/* GUI_PlaylistPreferences.cpp */

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


/* GUI_PlaylistPreferences.cpp */

#include "GUI_PlaylistPreferences.h"
#include "Gui/Preferences/ui_GUI_PlaylistPreferences.h"

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
	SetSetting(Set::PL_LoadSavedPlaylists, ui->cb_load_saved_playlists->isChecked() );
	SetSetting(Set::PL_LoadTemporaryPlaylists, ui->cb_load_temporary_playlists->isChecked() );
	SetSetting(Set::PL_LoadLastTrack, (ui->cb_load_last_track->isChecked() && ui->cb_load_last_track->isEnabled()) );
	SetSetting(Set::PL_RememberTime, (ui->cb_remember_time->isChecked() && ui->cb_remember_time->isEnabled()) );
	SetSetting(Set::PL_StartPlaying, (ui->cb_start_playing->isChecked() && ui->cb_start_playing->isEnabled()) );

	SetSetting(Set::PL_ShowNumbers, ui->cb_show_numbers->isChecked());
	SetSetting(Set::PL_ShowCovers, ui->cb_show_covers->isChecked());
	SetSetting(Set::PL_ShowRating, ui->cb_show_rating->isChecked());

	SetSetting(Set::PL_ShowClearButton, ui->cb_show_clear_button->isChecked());
	SetSetting(Set::PL_RememberTrackAfterStop, ui->cb_remember_after_stop->isChecked());

	if(evaluate_expression(ui->le_expression->text())){
		SetSetting(Set::PL_EntryLook, ui->le_expression->text());
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

	ui->cb_load_saved_playlists->setChecked(load_saved_playlists);
	ui->cb_load_temporary_playlists->setChecked(load_temporary_playlists);
	ui->cb_load_last_track->setChecked(load_last_track);
	ui->cb_remember_time->setChecked(remember_time);
	ui->cb_start_playing->setChecked(start_playing);

	ui->le_expression->setText(GetSetting(Set::PL_EntryLook));
	ui->cb_show_numbers->setChecked(GetSetting(Set::PL_ShowNumbers));
	ui->cb_show_covers->setChecked(GetSetting(Set::PL_ShowCovers));
	ui->cb_show_rating->setChecked(GetSetting(Set::PL_ShowRating));
	ui->cb_show_clear_button->setChecked(GetSetting(Set::PL_ShowClearButton));
	ui->cb_remember_after_stop->setChecked(GetSetting(Set::PL_RememberTrackAfterStop));
}


void GUI_PlaylistPreferences::init_ui()
{
	if(is_ui_initialized()){
		return;
	}

	setup_parent(this, &ui);
	ui->tabWidget->setCurrentIndex(0);
	ui->le_expression->setStyleSheet("font-family: mono;");

	revert();

	cb_toggled(true);

	connect(ui->cb_load_last_track, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::cb_toggled);
	connect(ui->cb_load_saved_playlists, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::cb_toggled);
	connect(ui->cb_load_temporary_playlists, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::cb_toggled);
	connect(ui->cb_remember_time, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::cb_toggled);
	connect(ui->cb_start_playing, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::cb_toggled);

	connect(ui->btn_default, &QPushButton::clicked, this, [=]()
	{
		ui->le_expression->setText("*%title%* - %artist%");
	});
}

QString GUI_PlaylistPreferences::action_name() const
{
	return Lang::get(Lang::Playlist);
}

void GUI_PlaylistPreferences::retranslate_ui()
{
	ui->retranslateUi(this);
	ui->lab_album->setText(Lang::get(Lang::Album));
	ui->lab_artist->setText(Lang::get(Lang::Artist));
	ui->lab_title->setText(Lang::get(Lang::Title));
	ui->lab_trackno->setText(Lang::get(Lang::TrackNo));
	ui->btn_default->setText(Lang::get(Lang::Default));
}

QString GUI_PlaylistPreferences::error_string() const
{
	return tr("Playlist look: Invalid expression");
}

void GUI_PlaylistPreferences::cb_toggled(bool b)
{
	Q_UNUSED(b);

	bool load = (ui->cb_load_saved_playlists->isChecked() || ui->cb_load_temporary_playlists->isChecked());

	ui->cb_load_last_track->setEnabled(load);
	ui->cb_remember_time->setEnabled(load);
	ui->cb_start_playing->setEnabled(load);

	bool cb_load_last_track_checked = ui->cb_load_last_track->isChecked() && ui->cb_load_last_track->isEnabled();
	ui->cb_remember_time->setEnabled(cb_load_last_track_checked);
}
