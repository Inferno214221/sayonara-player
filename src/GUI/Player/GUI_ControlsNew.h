/* GUI_ControlsNew.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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
	explicit GUI_ControlsNew(QWidget *parent=nullptr);
	~GUI_ControlsNew();

	// GUI_ControlsBase interface
public:
	QLabel* lab_sayonara() const override;
	QLabel* lab_title() const override;
	QLabel* lab_version() const override;
	QLabel* lab_album() const override;
	QLabel* lab_artist() const override;
	QLabel* lab_writtenby() const override;
	QLabel* lab_bitrate() const override;
	QLabel* lab_filesize() const override;
	QLabel* lab_copyright() const override;
	QLabel* lab_current_time() const override;
	QLabel* lab_max_time() const override;
	QWidget* widget_details() const override;
	RatingLabel* lab_rating() const override;
	SearchSlider* sli_progress() const override;
	SearchSlider* sli_volume() const override;
	Gui::ProgressBar* sli_buffer() const override;
	QPushButton* btn_mute() const override;
	QPushButton* btn_play() const override;
	QPushButton* btn_rec() const override;
	QPushButton* btn_bwd() const override;
	QPushButton* btn_fwd() const override;
	QPushButton* btn_stop() const override;
	CoverButton* btn_cover() const override;

	void toggle_buffer_mode(bool buffering) override;
	bool is_resizable() const override;

	void rating_changed_here(bool success);
};

#endif // GUI_CONTROLSNEW_H
