/* GUI_ControlsNew.cpp */

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

#include "GUI_ControlsNew.h"
#include "Gui/Player/ui_GUI_ControlsNew.h"
#include "Components/Tagging/UserTaggingOperations.h"
#include "Components/Tagging/ChangeNotifier.h"
#include "Components/PlayManager/PlayManager.h"
#include "Utils/MetaData/MetaDataList.h"

using Gui::RatingEditor;

GUI_ControlsNew::GUI_ControlsNew(QWidget* parent) :
	GUI_ControlsBase(parent)
{
	ui = new Ui::GUI_ControlsNew();
	ui->setupUi(this);

	ui->widget_rating->set_mousetrackable(false);
	connect(ui->widget_rating, &RatingEditor::sig_finished, this, &GUI_ControlsNew::rating_changed_here);
}

GUI_ControlsNew::~GUI_ControlsNew()
{
	delete ui; ui=nullptr;
}

QLabel* GUI_ControlsNew::lab_sayonara() const {	return ui->lab_sayonara; }
QLabel* GUI_ControlsNew::lab_title() const { return ui->lab_title; }
QLabel* GUI_ControlsNew::lab_version() const { return ui->lab_version; }
QLabel* GUI_ControlsNew::lab_album() const { return ui->lab_album; }
QLabel* GUI_ControlsNew::lab_artist() const { return ui->lab_artist; }
QLabel* GUI_ControlsNew::lab_writtenby() const { return ui->lab_writtenby; }
QLabel* GUI_ControlsNew::lab_bitrate() const { return ui->lab_bitrate; }
QLabel* GUI_ControlsNew::lab_copyright() const { return ui->lab_copyright; }
QLabel* GUI_ControlsNew::lab_filesize() const { return ui->lab_filesize; }
QLabel* GUI_ControlsNew::lab_current_time() const { return ui->lab_cur_time; }
QLabel* GUI_ControlsNew::lab_max_time() const { return ui->lab_max_time; }
QWidget* GUI_ControlsNew::widget_details() const { return ui->widget_details; }
Gui::RatingEditor* GUI_ControlsNew::lab_rating() const { return ui->widget_rating; }
Gui::SearchSlider* GUI_ControlsNew::sli_progress() const { return ui->sli_progress; }
Gui::SearchSlider* GUI_ControlsNew::sli_volume() const { return ui->sli_volume; }
QPushButton* GUI_ControlsNew::btn_mute() const { return ui->btn_mute; }
QPushButton* GUI_ControlsNew::btn_play() const { return ui->btn_ctrl_play; }
QPushButton* GUI_ControlsNew::btn_rec() const { return ui->btn_ctrl_rec; }
QPushButton* GUI_ControlsNew::btn_bwd() const { return ui->btn_ctrl_bw; }
QPushButton* GUI_ControlsNew::btn_fwd() const { return ui->btn_ctrl_fw; }
QPushButton* GUI_ControlsNew::btn_stop() const { return ui->btn_ctrl_stop; }
Gui::CoverButton* GUI_ControlsNew::btn_cover() const { return ui->btn_cover; }

void GUI_ControlsNew::rating_changed_here(bool save)
{
	MetaData md = PlayManager::instance()->current_track();

	if(!save)
	{
		ui->widget_rating->set_rating(md.rating());
		return;
	}

	Rating rating = ui->widget_rating->rating();


	Tagging::UserOperations* uto = new Tagging::UserOperations(md.library_id, this);
	connect(uto, &Tagging::UserOperations::sig_finished, uto, &QObject::deleteLater);
	uto->set_track_rating(md, rating);
}
bool GUI_ControlsNew::is_extern_resize_allowed() const
{
	return true;
}

void GUI_ControlsNew::language_changed()
{
	if(ui){
		ui->retranslateUi(this);
	}
}
