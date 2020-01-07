/* GUI_Controls.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef GUI_CONTROLS_H
#define GUI_CONTROLS_H

#include "GUI_ControlsBase.h"

UI_FWD(GUI_Controls)

class MetaData;
class MetaDataList;

class GUI_Controls :
		public GUI_ControlsBase
{
	Q_OBJECT
	UI_CLASS(GUI_Controls)

public:
	explicit GUI_Controls(QWidget *parent=nullptr);
	~GUI_Controls();

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
	Gui::SearchSlider* sli_progress() const override;
	Gui::SearchSlider* sli_volume() const override;
	QPushButton* btn_mute() const override;
	QPushButton* btn_play() const override;
	QPushButton* btn_rec() const override;
	QPushButton* btn_bwd() const override;
	QPushButton* btn_fwd() const override;
	QPushButton* btn_stop() const override;
	Gui::CoverButton* btn_cover() const override;

	bool is_extern_resize_allowed() const override;

protected:
	void language_changed() override;
};



#endif // GUI_CONTROLS_H
