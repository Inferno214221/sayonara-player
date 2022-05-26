/* GUI_ControlsNew.h */

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

#ifndef GUI_CONTROLSNEW_H
#define GUI_CONTROLSNEW_H

#include "GUI_ControlsBase.h"

UI_FWD(GUI_ControlsNew)

class MetaData;
class MetaDataList;
class PlayManager;

class GUI_ControlsNew :
	public GUI_ControlsBase
{
	Q_OBJECT
	PIMPL(GUI_ControlsNew)
	UI_CLASS_SHARED_PTR(GUI_ControlsNew)

	public:
		explicit GUI_ControlsNew(PlayManager* playManager, CoverDataProvider* coverProvider, QWidget* parent = nullptr);
		~GUI_ControlsNew() override;

		[[nodiscard]] QLabel* labSayonara() const override;
		[[nodiscard]] QLabel* labTitle() const override;
		[[nodiscard]] QLabel* labVersion() const override;
		[[nodiscard]] QLabel* labAlbum() const override;
		[[nodiscard]] QLabel* labArtist() const override;
		[[nodiscard]] QLabel* labWrittenBy() const override;
		[[nodiscard]] QLabel* labBitrate() const override;
		[[nodiscard]] QLabel* labFilesize() const override;
		[[nodiscard]] QLabel* labCopyright() const override;
		[[nodiscard]] QLabel* labCurrentTime() const override;
		[[nodiscard]] QLabel* labMaxTime() const override;
		[[nodiscard]] QWidget* widgetDetails() const override;
		[[nodiscard]] Gui::SearchSlider* sliProgress() const override;
		[[nodiscard]] Gui::SearchSlider* sliVolume() const override;
		[[nodiscard]] QPushButton* btnMute() const override;
		[[nodiscard]] QPushButton* btnPlay() const override;
		[[nodiscard]] QPushButton* btnRecord() const override;
		[[nodiscard]] QPushButton* btnPrevious() const override;
		[[nodiscard]] QPushButton* btnNext() const override;
		[[nodiscard]] QPushButton* btnStop() const override;
		[[nodiscard]] Gui::CoverButton* btnCover() const override;
		[[nodiscard]] Gui::RatingEditor* labRating() const override;

		[[nodiscard]] bool isExternResizeAllowed() const override;

		void ratingChangedHere(bool save);

	protected:
		void languageChanged() override;
};

#endif // GUI_CONTROLSNEW_H
