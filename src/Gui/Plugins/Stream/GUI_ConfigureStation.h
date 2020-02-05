/* GUI_ConfigureStation.h */

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

#ifndef GUI_ConfigureStation_H
#define GUI_ConfigureStation_H

#include "Gui/Utils/Widgets/Dialog.h"

#include "Utils/Pimpl.h"
#include "Utils/Streams/Station.h"

UI_FWD(GUI_ConfigureStation)

class GUI_ConfigureStation :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_ConfigureStation)
	UI_CLASS(GUI_ConfigureStation)

public:
	enum Mode
	{
		New,
		Edit
	};

	GUI_ConfigureStation(QWidget* parent=nullptr);
	virtual ~GUI_ConfigureStation();

	virtual StationPtr configured_station() = 0;
	virtual QList<QWidget*> configuration_widgets(StationPtr station) = 0;
	virtual QString label_text(int row) const=0;

	virtual void init_ui(StationPtr station);

	void set_error_message(const QString& message);

	void set_mode(const QString& type, Mode mode);
	bool was_accepted() const;
};

#endif // GUI_ConfigureStation_H
