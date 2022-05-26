/* SearchSlider.h

 * Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
 * created by Michael Lugmair (Lucio Carreras),
 * Sep 14, 2012
 *
 */

#ifndef SAYONARA_SEARCHSLIDER_H
#define SAYONARA_SEARCHSLIDER_H

#include "Gui/Utils/Widgets/Slider.h"

class QColor;
namespace Gui
{
	class SearchSlider :
		public Gui::Slider
	{
		Q_OBJECT

		signals:
			void sigSliderMoved(int);

		public:
			[[maybe_unused]] explicit SearchSlider(QWidget* parent = nullptr);
			~SearchSlider() override;

			[[nodiscard]] bool isBusy() const;

		protected:
			void mousePressEvent(QMouseEvent* e) override;
			void mouseReleaseEvent(QMouseEvent* e) override;
			void mouseMoveEvent(QMouseEvent* e) override;
			bool event(QEvent* event) override;

		private:
			void emitNewValue(int value);
	};
}

#endif /* SAYONARA_SEARCHSLIDER_H */
