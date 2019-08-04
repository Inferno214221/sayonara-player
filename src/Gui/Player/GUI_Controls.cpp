/* GUI_Controls.cpp */

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

#include "GUI_Controls.h"
#include "Gui/Player/ui_GUI_Controls.h"

GUI_Controls::GUI_Controls(QWidget* parent) :
	GUI_ControlsBase(parent)
{
	ui = new Ui::GUI_Controls();
	ui->setupUi(this);
}

GUI_Controls::~GUI_Controls()
{
	delete ui; ui=nullptr;
}

QLabel* GUI_Controls::lab_sayonara() const {	return ui->lab_sayonara; }
QLabel* GUI_Controls::lab_title() const { return ui->lab_title; }
QLabel* GUI_Controls::lab_version() const { return ui->lab_version; }
QLabel* GUI_Controls::lab_album() const { return ui->lab_album; }
QLabel* GUI_Controls::lab_artist() const { return ui->lab_artist; }
QLabel* GUI_Controls::lab_writtenby() const { return ui->lab_writtenby; }
QLabel* GUI_Controls::lab_bitrate() const { return ui->lab_bitrate; }
QLabel* GUI_Controls::lab_filesize() const { return ui->lab_filesize; }
QLabel* GUI_Controls::lab_copyright() const { return ui->lab_copyright; }
QLabel* GUI_Controls::lab_current_time() const { return ui->lab_cur_time; }
QLabel* GUI_Controls::lab_max_time() const { return ui->lab_max_time; }
QWidget* GUI_Controls::widget_details() const { return ui->widget_details; }
Gui::SearchSlider* GUI_Controls::sli_progress() const { return ui->sli_progress; }
Gui::SearchSlider* GUI_Controls::sli_volume() const { return ui->sli_volume; }
QPushButton* GUI_Controls::btn_mute() const { return ui->btn_mute; }
QPushButton* GUI_Controls::btn_play() const { return ui->btn_ctrl_play; }
QPushButton* GUI_Controls::btn_rec() const { return ui->btn_ctrl_rec; }
QPushButton* GUI_Controls::btn_bwd() const { return ui->btn_ctrl_bw; }
QPushButton* GUI_Controls::btn_fwd() const { return ui->btn_ctrl_fw; }
QPushButton* GUI_Controls::btn_stop() const { return ui->btn_ctrl_stop; }
Gui::CoverButton* GUI_Controls::btn_cover() const { return ui->btn_cover; }

bool GUI_Controls::is_extern_resize_allowed() const
{
	return false;
}

void GUI_Controls::language_changed()
{
	if(ui){
		ui->retranslateUi(this);
	}
}
