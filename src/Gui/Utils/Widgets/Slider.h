/* Slider.h */

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

#ifndef SAYONARA_SLIDER_H
#define SAYONARA_SLIDER_H

#include "Utils/Pimpl.h"

#include <QMap>
#include <QSlider>

class QColor;

namespace Gui
{
	/**
	 * @brief Dont use it directly but inherit it
	 * @ingroup Widgets
	 */
	class Slider :
			public QSlider
	{
		Q_OBJECT
		PIMPL(Slider)

		signals:
			void sigSliderGotFocus();
			void sigSliderLostFocus();
			void sigSliderHovered(int);

		public:
			explicit Slider(QWidget* parent=nullptr);
			virtual ~Slider() override;

		protected:
			void sliderChange(SliderChange change) override;
			int valueFromPosition(const QPoint& pos) const;

			virtual void focusInEvent(QFocusEvent* e) override;
			virtual void focusOutEvent(QFocusEvent* e) override;
			virtual void mousePressEvent(QMouseEvent* e) override;
			virtual void mouseMoveEvent(QMouseEvent* e) override;
			virtual void mouseReleaseEvent(QMouseEvent* e) override;

			virtual bool hasAdditionalValue() const;
			virtual int additionalValue() const;
			virtual QColor additionalValueColor() const;

			virtual bool event(QEvent* e) override;
			void paintEvent(QPaintEvent* e) override;
	};

}
#endif // SAYONARA_SLIDER_H
