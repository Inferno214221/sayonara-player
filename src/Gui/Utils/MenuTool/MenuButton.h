/* MenuButton.h */

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

#ifndef MENUBUTTON_H
#define MENUBUTTON_H

#include <QPushButton>
#include "Gui/Utils/Widgets/WidgetTemplate.h"


namespace Gui
{
	/**
	 * @brief The MenuButton class.\n
	 * A button that sends a signal when clicked. \n
	 * This Class is meant for inheritance like MenuToolButton does.
	 * @ingroup GUIHelper
	 */
	class MenuButton :
			public Gui::WidgetTemplate<QPushButton>
	{
		Q_OBJECT

		signals:
			void sig_triggered(QPoint);

		protected:
			virtual void mousePressEvent(QMouseEvent* e) override;

			/**
			 * @brief subclasses should call this function and show their menu
			 * @param pos position of mouse pointer
			 */
			virtual void show_menu(QPoint pos);

			/**
			 * @brief subclasses should reimplement this function.\n
			 * This method should return false if the button should be disabled and false else
			 * @return true if button should be enabled, false else
			 */
			virtual bool prove_enabled();

			void skin_changed() override;
			void language_changed() override;

		public:
			explicit MenuButton(QWidget* parent=nullptr);
			virtual ~MenuButton();
	};
}

#endif // MENUBUTTON_H
