/* FloatingLabel.h */

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



#ifndef LABEL_H
#define LABEL_H

#include "Utils/Pimpl.h"
#include <QLabel>
#include "Gui/Utils/Widgets/WidgetTemplate.h"

class QTimer;
class QString;
namespace Gui
{
	/**
	 * @brief The FloatingLabel class. A QLabel where the text floats
	 * from left to right and vice versa so there's no need for word wrapping anymore.
	 */
	class FloatingLabel :
		public Gui::WidgetTemplate<QLabel>
	{
		Q_OBJECT
		PIMPL(FloatingLabel)

		public:
			explicit FloatingLabel(QWidget* parent=nullptr);
			~FloatingLabel();

			void setFloatingText(const QString& text);
			void setCharsPerSecond(int charsPerSecond);

		public slots:
			void updateOffset();

		protected:
			void paintEvent(QPaintEvent* event);
			void resizeEvent(QResizeEvent* event);
	};
}

#endif // LABEL_H
