/* GUI_Controls.cpp */

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

#include "GUI_Controls.h"
#include "Gui/Player/ui_GUI_Controls.h"

GUI_Controls::GUI_Controls(PlayManager* playManager, QWidget* parent) :
	GUI_ControlsBase(playManager, parent)
{
	ui = new Ui::GUI_Controls();
	ui->setupUi(this);
}

GUI_Controls::~GUI_Controls()
{
	delete ui;
	ui = nullptr;
}

QLabel* GUI_Controls::labSayonara() const { return ui->lab_sayonara; }

QLabel* GUI_Controls::labTitle() const { return ui->lab_title; }

QLabel* GUI_Controls::labVersion() const { return ui->lab_version; }

QLabel* GUI_Controls::labAlbum() const { return ui->lab_album; }

QLabel* GUI_Controls::labArtist() const { return ui->lab_artist; }

QLabel* GUI_Controls::labWrittenBy() const { return ui->lab_writtenby; }

QLabel* GUI_Controls::labBitrate() const { return ui->lab_bitrate; }

QLabel* GUI_Controls::labFilesize() const { return ui->lab_filesize; }

QLabel* GUI_Controls::labCopyright() const { return ui->lab_copyright; }

QLabel* GUI_Controls::labCurrentTime() const { return ui->lab_cur_time; }

QLabel* GUI_Controls::labMaxTime() const { return ui->lab_max_time; }

QWidget* GUI_Controls::widgetDetails() const { return ui->widget_details; }

Gui::SearchSlider* GUI_Controls::sliProgress() const { return ui->sli_progress; }

Gui::SearchSlider* GUI_Controls::sliVolume() const { return ui->sli_volume; }

QPushButton* GUI_Controls::btnMute() const { return ui->btn_mute; }

QPushButton* GUI_Controls::btnPlay() const { return ui->btn_ctrl_play; }

QPushButton* GUI_Controls::btnRecord() const { return ui->btn_ctrl_rec; }

QPushButton* GUI_Controls::btnPrevious() const { return ui->btn_ctrl_bw; }

QPushButton* GUI_Controls::btnNext() const { return ui->btn_ctrl_fw; }

QPushButton* GUI_Controls::btnStop() const { return ui->btn_ctrl_stop; }

Gui::CoverButton* GUI_Controls::btnCover() const { return ui->btn_cover; }

bool GUI_Controls::isExternResizeAllowed() const
{
	return false;
}

void GUI_Controls::languageChanged()
{
	if(ui)
	{
		ui->retranslateUi(this);
	}
}
