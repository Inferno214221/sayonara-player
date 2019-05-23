/* GUI_Podcasts.h */

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

#ifndef GUI_PODCASTS_H
#define GUI_PODCASTS_H

#include "GUI_AbstractStream.h"

UI_FWD(GUI_Podcasts)

class GUI_Podcasts :
		public GUI_AbstractStream
{
	Q_OBJECT
	UI_CLASS(GUI_Podcasts)

public:
	explicit GUI_Podcasts(QWidget *parent=nullptr);
	virtual ~GUI_Podcasts();

	QString get_name() const override;
	QString get_display_name() const override;


private:
	void init_ui() override;
	void retranslate_ui() override;
	QString get_title_fallback_name() const override;

	// GUI_AbstractStream interface
	protected:
	QComboBox* combo_stream() override;
	QPushButton* btn_play() override;
	MenuToolButton* btn_menu() override;

	AbstractStreamHandler* stream_handler() const override;
};

#endif // GUI_PODCASTS_H
