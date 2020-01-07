/* SearchSlider.h

 * Copyright (C) 2011-2020 Lucio Carreras
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Lucio Carreras,
 * Sep 14, 2012
 *
 */

#ifndef SEARCHSLIDER_H_
#define SEARCHSLIDER_H_

#include "Gui/Utils/Widgets/Slider.h"

class QColor;
namespace Gui
{
	/**
	 * @brief A slider as it is used by the progress bar
	 * You can also set a different value by calling set_buffering
	 * which is displayed in a different color when using the
	 * dark skin
	 */
	class SearchSlider:
			public Gui::Slider
	{
		Q_OBJECT
		PIMPL(SearchSlider)

	signals:
		void sig_slider_moved(int);

	public:
		explicit SearchSlider(QWidget* parent=nullptr);
		virtual ~SearchSlider();

		/**
		 * @brief Returns true if it's actually moved by the user
		 * @return
		 */
		bool is_busy() const;

		/**
		 * @brief Set a second value beside QSlider::setValue() which
		 * is displayed in another color
		 * @param progress
		 */
		void set_buffering(int progress);

	protected:
		void mousePressEvent(QMouseEvent* e) override;
		void mouseReleaseEvent(QMouseEvent* e) override;
		void mouseMoveEvent(QMouseEvent* e) override;
		bool event(QEvent *event) override;

		bool has_other_value() const override;
		int other_value() const override;
		QColor other_value_color() const override;


	private:
		void emit_new_val(int value);
	};
}

#endif /* SEARCHSLIDER_H_ */
