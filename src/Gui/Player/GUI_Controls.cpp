/* GUI_Controls.cpp */

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

#include "GUI_Controls.h"
#include "Gui/Player/ui_GUI_Controls.h"

GUI_Controls::GUI_Controls(PlayManager* playManager, CoverDataProvider* coverProvider, QWidget* parent) :
	GUI_ControlsBase(playManager, coverProvider, parent),
	ui(std::make_shared<Ui::GUI_Controls>())
{
	ui->setupUi(this);
}

GUI_Controls::~GUI_Controls() = default;

QLabel* GUI_Controls::labSayonara() const { return ui->labSayonara; }

QLabel* GUI_Controls::labTitle() const { return ui->labTitle; }

QLabel* GUI_Controls::labVersion() const { return ui->labVersion; }

QLabel* GUI_Controls::labAlbum() const { return ui->labAlbum; }

QLabel* GUI_Controls::labArtist() const { return ui->labArtist; }

QLabel* GUI_Controls::labWrittenBy() const { return ui->labWrittenBy; }

QLabel* GUI_Controls::labBitrate() const { return ui->labBitrate; }

QLabel* GUI_Controls::labFilesize() const { return ui->labFilesize; }

QLabel* GUI_Controls::labCopyright() const { return ui->labCopyright; }

QLabel* GUI_Controls::labCurrentTime() const { return ui->labCurrentTime; }

QLabel* GUI_Controls::labMaxTime() const { return ui->labDuration; }

QWidget* GUI_Controls::widgetDetails() const { return ui->widgetDetails; }

Gui::SearchSlider* GUI_Controls::sliProgress() const { return ui->sliProgress; }

Gui::SearchSlider* GUI_Controls::sliVolume() const { return ui->sliVolume; }

QPushButton* GUI_Controls::btnMute() const { return ui->btnMute; }

QPushButton* GUI_Controls::btnPlay() const { return ui->btnPlay; }

QPushButton* GUI_Controls::btnRecord() const { return ui->btnRec; }

QPushButton* GUI_Controls::btnPrevious() const { return ui->btnPrev; }

QPushButton* GUI_Controls::btnNext() const { return ui->btnNext; }

QPushButton* GUI_Controls::btnStop() const { return ui->btnStop; }

Gui::CoverButton* GUI_Controls::btnCover() const { return ui->btnCover; }

bool GUI_Controls::isExternResizeAllowed() const { return false; }

void GUI_Controls::languageChanged() { ui->retranslateUi(this); }
