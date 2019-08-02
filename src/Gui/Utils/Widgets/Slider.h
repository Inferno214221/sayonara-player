/* Slider.h */

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

#ifndef SAYONARA_SLIDER_H
#define SAYONARA_SLIDER_H

#include "Utils/Pimpl.h"

#include <QMap>
#include <QSlider>

class QColor;

namespace Gui
{
	/**
	 * @brief The Slider class
	 * @ingroup Widgets
	 */
	class Slider :
			public QSlider
	{
		Q_OBJECT
		PIMPL(Slider)

		signals:
			void sig_slider_got_focus();
			void sig_slider_lost_focus();
			void sig_slider_hovered(int);

		public:
			explicit Slider(QWidget* parent=nullptr);
			virtual ~Slider();

		protected:
			void sliderChange(SliderChange change) override;
			int get_val_from_pos(const QPoint& pos) const;

			virtual void focusInEvent(QFocusEvent* e) override;
			virtual void focusOutEvent(QFocusEvent* e) override;
			virtual void mousePressEvent(QMouseEvent* e) override;
			virtual void mouseMoveEvent(QMouseEvent* e) override;
			virtual void mouseReleaseEvent(QMouseEvent* e) override;

			virtual bool has_other_value() const;
			virtual int other_value() const;
			virtual QColor other_value_color() const;

			virtual bool event(QEvent* e) override;
			void paintEvent(QPaintEvent *e) override;
	};

}
#endif // SAYONARA_SLIDER_H
