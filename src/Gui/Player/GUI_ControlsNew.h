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

class GUI_ControlsNew :
		public GUI_ControlsBase
{
	Q_OBJECT
	UI_CLASS(GUI_ControlsNew)

	public:
		explicit GUI_ControlsNew(QWidget* parent=nullptr);
		~GUI_ControlsNew() override;

		// GUI_ControlsBase interface
	public:
		QLabel* labSayonara() const override;
		QLabel* labTitle() const override;
		QLabel* labVersion() const override;
		QLabel* labAlbum() const override;
		QLabel* labArtist() const override;
		QLabel* labWrittenBy() const override;
		QLabel* labBitrate() const override;
		QLabel* labFilesize() const override;
		QLabel* labCopyright() const override;
		QLabel* labCurrentTime() const override;
		QLabel* labMaxTime() const override;
		QWidget* widgetDetails() const override;
		Gui::RatingEditor* labRating() const override;
		Gui::SearchSlider* sliProgress() const override;
		Gui::SearchSlider* sliVolume() const override;
		QPushButton* btnMute() const override;
		QPushButton* btnPlay() const override;
		QPushButton* btnRecord() const override;
		QPushButton* btnPrevious() const override;
		QPushButton* btnNext() const override;
		QPushButton* btnStop() const override;
		Gui::CoverButton* btnCover() const override;

		void ratingChangedHere(bool save);
		bool isExternResizeAllowed() const override;

	protected:
		void languageChanged() override;
};

#endif // GUI_CONTROLSNEW_H
