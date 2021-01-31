/* GUI_AudioConverter.h */

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

#ifndef GUI_AUDIOCONVERTER_H
#define GUI_AUDIOCONVERTER_H

#include "Utils/Pimpl.h"
#include "Gui/Plugins/PlayerPluginBase.h"
#include "Components/PlayManager/PlayState.h"

UI_FWD(GUI_AudioConverter)

class ConverterFactory;

class GUI_AudioConverter :
		public PlayerPlugin::Base
{
	Q_OBJECT
	UI_CLASS(GUI_AudioConverter)
	PIMPL(GUI_AudioConverter)

	public:
		explicit GUI_AudioConverter(ConverterFactory* converterFactory, QWidget* parent=nullptr);
		virtual ~GUI_AudioConverter() override;

		QString	name() const override;
		QString	displayName() const override;

	private slots:
		void btnStartClicked();
		void convertFinished();
		void comboCodecsIndexChanged(int idx);
		void resetButtons();

		void oggQualityChanged(int value);
		void comboLameCbrIndexChanged(int idx);
		void comboLameVbrIndexChanged(int idx);

		void threadCountChanged(int value);

	private:
		void checkStartButton();

		void retranslate() override;
		void initUi() override;
};


#endif // GUI_AUDIOCONVERTER_H
