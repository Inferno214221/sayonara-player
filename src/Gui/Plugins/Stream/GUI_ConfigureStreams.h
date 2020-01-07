/* GUI_ConfigureStreams.h */

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



#ifndef GUI_CONFIGURESTREAMS_H
#define GUI_CONFIGURESTREAMS_H

#include <QObject>
#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_ConfigureStreams)

class GUI_ConfigureStreams :
	public Gui::Dialog
{
	Q_OBJECT
	UI_CLASS(GUI_ConfigureStreams)

public:

	enum Mode
	{New, Edit};

	GUI_ConfigureStreams(const QString& type, Mode mode, QWidget* parent=nullptr);
	~GUI_ConfigureStreams();

	QString url() const;
	QString name() const;

	void set_url(const QString& url);
	void set_name(const QString& name);
	void set_error_message(const QString& message);

	void set_mode(const QString& trype, Mode mode);
	bool was_accepted() const;
};

#endif // GUI_CONFIGURESTREAMS_H
