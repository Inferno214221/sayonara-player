/* GUI_ControlsNew.cpp */

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

	ui->widget_rating->setMouseTrackable(false);
	connect(ui->widget_rating, &RatingEditor::sigFinished, this, &GUI_ControlsNew::ratingChangedHere);
}

GUI_ControlsNew::~GUI_ControlsNew()
{
	delete ui;
	ui = nullptr;
}

QLabel* GUI_ControlsNew::labSayonara() const { return ui->lab_sayonara; }

QLabel* GUI_ControlsNew::labTitle() const { return ui->lab_title; }

QLabel* GUI_ControlsNew::labVersion() const { return ui->lab_version; }

QLabel* GUI_ControlsNew::labAlbum() const { return ui->lab_album; }

QLabel* GUI_ControlsNew::labArtist() const { return ui->lab_artist; }

QLabel* GUI_ControlsNew::labWrittenBy() const { return ui->lab_writtenby; }

QLabel* GUI_ControlsNew::labBitrate() const { return ui->lab_bitrate; }

QLabel* GUI_ControlsNew::labCopyright() const { return ui->lab_copyright; }

QLabel* GUI_ControlsNew::labFilesize() const { return ui->lab_filesize; }

QLabel* GUI_ControlsNew::labCurrentTime() const { return ui->lab_cur_time; }

QLabel* GUI_ControlsNew::labMaxTime() const { return ui->lab_max_time; }

QWidget* GUI_ControlsNew::widgetDetails() const { return ui->widget_details; }

Gui::RatingEditor* GUI_ControlsNew::labRating() const { return ui->widget_rating; }

Gui::SearchSlider* GUI_ControlsNew::sliProgress() const { return ui->sli_progress; }

Gui::SearchSlider* GUI_ControlsNew::sliVolume() const { return ui->sli_volume; }

QPushButton* GUI_ControlsNew::btnMute() const { return ui->btn_mute; }

QPushButton* GUI_ControlsNew::btnPlay() const { return ui->btn_ctrl_play; }

QPushButton* GUI_ControlsNew::btnRecord() const { return ui->btn_ctrl_rec; }

QPushButton* GUI_ControlsNew::btnPrevious() const { return ui->btn_ctrl_bw; }

QPushButton* GUI_ControlsNew::btnNext() const { return ui->btn_ctrl_fw; }

QPushButton* GUI_ControlsNew::btnStop() const { return ui->btn_ctrl_stop; }

Gui::CoverButton* GUI_ControlsNew::btnCover() const { return ui->btn_cover; }

void GUI_ControlsNew::ratingChangedHere(bool save)
{
	const auto& md = PlayManagerProvider::instance()->playManager()->currentTrack();

	if(!save)
	{
		ui->widget_rating->setRating(md.rating());
		return;
	}

	Rating rating = ui->widget_rating->rating();

	auto* uto = new Tagging::UserOperations(md.libraryId(), this);
	connect(uto, &Tagging::UserOperations::sigFinished, uto, &QObject::deleteLater);
	uto->setTrackRating(md, rating);
}

bool GUI_ControlsNew::isExternResizeAllowed() const
{
	return true;
}

void GUI_ControlsNew::languageChanged()
{
	if(ui)
	{
		ui->retranslateUi(this);
	}
}
