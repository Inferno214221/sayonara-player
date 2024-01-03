/* EqualizerSlider.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef EQSLIDER_H
#define EQSLIDER_H

#include "Gui/Utils/Widgets/Slider.h"
#include "Utils/Pimpl.h"

class QLabel;

namespace Gui
{
	/**
	 * @brief The EqSlider class
	 * @ingroup Equalizer
	 */
	class EqualizerSlider :
		public Gui::Slider
	{
		Q_OBJECT
		PIMPL(EqualizerSlider)

		signals:
			void sigValueChanged(int idx, int val);

		public:
			explicit EqualizerSlider(QWidget* parent);
			~EqualizerSlider() override;

			/**
			 * @brief sets everything the slider has to be aware about
			 * @param idx the index of the slider
			 * @param label the value label of the slider
			 */
			void setLabel(int idx, QLabel* label);

			/**
			 * @brief get the value label
			 * @return
			 */
			QLabel* label() const;

			/**
			 * @brief get the index of the slider
			 * @return
			 */
			int index() const;

			/**
			 * @brief get the gstreamer compatible value
			 * @return
			 */
			double equalizerValue() const;
			void setEqualizerValue(double value);

			QSize minimumSizeHint() const override;

			void setSilent(bool b);

		private slots:
			void setZero();

		protected:
			void sliderChange(SliderChange change) override;
			using Gui::Slider::value;
			using Gui::Slider::setValue;
	};
}

#endif // EQSLIDER_H
