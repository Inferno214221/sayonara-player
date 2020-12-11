/* GUI_Equalizer.h */

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


/*
 * GUI_Equalizer.h
 *
 *  Created on: May 18, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
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
		~GUI_Equalizer() override;

		QString name() const override;
		QString displayName() const override;

	public slots:
		void fillEqualizerPresets();

	private:
		void initUi() override;
		void retranslate() override;

	private slots:
		void sliderValueChanged(int band, int value);
		void sliderPressed();
		void sliderReleased();

		void valueChanged(int band, int value);

		void currentEqualizerChanged(int index);
		void checkboxGaussToggled(bool);

		void btnDefaultClicked();
		void btnSaveAsClicked();
		void btnDeleteClicked();
		void btnRenameClicked();

		void saveAsOkClicked();
		void renameOkClicked();
};

#endif /* GUI_EQUALIZER_H_ */
