/* GUI_Equalizer.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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


/*
 * GUI_Equalizer.h
 *
 *  Created on: May 18, 2011
 *      Author: Lucio Carreras
 */

#ifndef GUI_EQUALIZER_H_
#define GUI_EQUALIZER_H_

#include "Gui/Plugins/PlayerPluginBase.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_Equalizer)

/**
 * @brief The GUI_Equalizer class
 * @ingroup Equalizer
 */
class GUI_Equalizer :
		public PlayerPlugin::Base
{
	Q_OBJECT
	UI_CLASS(GUI_Equalizer)
	PIMPL(GUI_Equalizer)

public:
	explicit GUI_Equalizer(QWidget* parent=nullptr);
	virtual ~GUI_Equalizer();

	QString get_name() const override;
	QString get_display_name() const override;

public slots:
	void fill_eq_presets();

private:
	void init_ui() override;
	void retranslate_ui() override;
	void save_current_preset(const QString& name);

private slots:
	void sli_changed(int idx, int value);
	void sli_pressed();
	void sli_released();

	void preset_changed(int);
	void cb_gauss_toggled(bool);

	void btn_default_clicked();
	void btn_save_clicked();
	void btn_save_as_clicked();
	void btn_delete_clicked();
	void btn_undo_clicked();

	void save_as_ok_clicked();
};

#endif /* GUI_EQUALIZER_H_ */
