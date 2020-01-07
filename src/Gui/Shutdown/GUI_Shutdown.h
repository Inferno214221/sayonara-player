/* GUI_Shutdown.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef GUI_SHUTDOWN_H
#define GUI_SHUTDOWN_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Gui/Utils/GuiClass.h"
#include "Utils/Macros.h"

#ifdef SAYONARA_WITH_SHUTDOWN

UI_FWD(GUI_Shutdown)

class GUI_Shutdown :
		public Gui::Dialog
{
	Q_OBJECT
	UI_CLASS(GUI_Shutdown)

	private slots:
		void ok_clicked();
		void cancel_clicked();
		void rb_after_finished_clicked(bool b);
		void rb_after_minutes_clicked(bool b);

	public:
		explicit GUI_Shutdown(QWidget* parent=nullptr);
		~GUI_Shutdown();

	protected:
		void skin_changed();
};

#endif // SAYONARA_WITH_SHUTDOWN

#endif // GUI_SHUTDOWN_H
