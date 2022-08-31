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

#include "Components/PlayManager/PlayManager.h"
#include "Components/Tagging/UserTaggingOperations.h"
#include "Utils/MetaData/MetaDataList.h"

using Gui::RatingEditor;

struct GUI_ControlsNew::Private
{
	PlayManager* playManager;

	explicit Private(PlayManager* playManager) :
		playManager {playManager} {}
};

GUI_ControlsNew::GUI_ControlsNew(PlayManager* playManager, CoverDataProvider* coverProvider, QWidget* parent) :
	GUI_ControlsBase(playManager, coverProvider, parent)
{
	m = Pimpl::make<Private>(playManager);

	ui = std::make_shared<Ui::GUI_ControlsNew>();
	ui->setupUi(this);

	constexpr const auto ScaleFactor = 1.5;
	constexpr const auto MaxLines = 5;
	ui->widgetRating->setMaximumHeight(static_cast<int>(this->fontMetrics().height() * ScaleFactor));
	ui->widgetRating->setMaximumWidth(ui->widgetRating->height() * MaxLines);

	ui->widgetRating->setMouseTrackable(false);
	connect(ui->widgetRating, &RatingEditor::sigFinished, this, &GUI_ControlsNew::ratingChangedHere);
}

GUI_ControlsNew::~GUI_ControlsNew() = default;

QLabel* GUI_ControlsNew::labSayonara() const { return ui->labSayonara; }

QLabel* GUI_ControlsNew::labTitle() const { return ui->labTitle; }

QLabel* GUI_ControlsNew::labVersion() const { return ui->labVersion; }

QLabel* GUI_ControlsNew::labAlbum() const { return ui->labAlbum; }

QLabel* GUI_ControlsNew::labArtist() const { return ui->labArtist; }

QLabel* GUI_ControlsNew::labWrittenBy() const { return ui->labWrittenBy; }

QLabel* GUI_ControlsNew::labBitrate() const { return ui->labBitrate; }

QLabel* GUI_ControlsNew::labCopyright() const { return ui->labCopyright; }

QLabel* GUI_ControlsNew::labFilesize() const { return ui->labFilesize; }

QLabel* GUI_ControlsNew::labCurrentTime() const { return ui->labCurrentTime; }

QLabel* GUI_ControlsNew::labMaxTime() const { return ui->labDuration; }

QWidget* GUI_ControlsNew::widgetDetails() const { return ui->widgetDetails; }

Gui::RatingEditor* GUI_ControlsNew::labRating() const { return ui->widgetRating; }

Gui::SearchSlider* GUI_ControlsNew::sliProgress() const { return ui->sliProgress; }

Gui::SearchSlider* GUI_ControlsNew::sliVolume() const { return ui->sliVolume; }

QPushButton* GUI_ControlsNew::btnMute() const { return ui->btnMute; }

QPushButton* GUI_ControlsNew::btnPlay() const { return ui->btnPlay; }

QPushButton* GUI_ControlsNew::btnRecord() const { return ui->btnRec; }

QPushButton* GUI_ControlsNew::btnPrevious() const { return ui->btnPrev; }

QPushButton* GUI_ControlsNew::btnNext() const { return ui->btnNext; }

QPushButton* GUI_ControlsNew::btnStop() const { return ui->btnStop; }

Gui::CoverButton* GUI_ControlsNew::btnCover() const { return ui->btnCover; }

void GUI_ControlsNew::ratingChangedHere(bool save)
{
	const auto& track = m->playManager->currentTrack();

	if(!save)
	{
		ui->widgetRating->setRating(track.rating());
		return;
	}

	const auto rating = ui->widgetRating->rating();

	auto* uto = new Tagging::UserOperations(track.libraryId(), this);
	connect(uto, &Tagging::UserOperations::sigFinished, uto, &QObject::deleteLater);
	uto->setTrackRating(track, rating);
}

bool GUI_ControlsNew::isExternResizeAllowed() const { return true; }

void GUI_ControlsNew::languageChanged()
{
	ui->retranslateUi(this);
}
