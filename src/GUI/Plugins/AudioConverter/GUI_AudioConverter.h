/* GUI_AudioConverter.h */

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

#ifndef GUI_AUDIOCONVERTER_H
#define GUI_AUDIOCONVERTER_H

#include "Utils/Pimpl.h"
#include "Interfaces/PlayerPlugin/PlayerPluginBase.h"
#include "Components/PlayManager/PlayState.h"

UI_FWD(GUI_AudioConverter)

class Converter;
class GUI_AudioConverter :
		public PlayerPlugin::Base
{
	Q_OBJECT
	UI_CLASS(GUI_AudioConverter)
	PIMPL(GUI_AudioConverter)

	public:
		explicit GUI_AudioConverter(QWidget *parent=nullptr);
		virtual ~GUI_AudioConverter() override;

		QString	get_name() const override;
		QString	get_display_name() const override;

	private slots:
		void btn_start_clicked();
		void convert_finished();
		void combo_codecs_changed(int idx);
		void reset_buttons();

		void ogg_quality_changed(int value);
		void combo_cbr_lame_changed(int idx);
		void lame_vbr_changed(int idx);

		void num_threads_changed(int value);

	private:
		void check_start_button();
		Converter* create_converter();

		void retranslate_ui() override;
		void init_ui() override;
};


#endif // GUI_AUDIOCONVERTER_H
