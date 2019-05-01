/* LibraryPluginMenu.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#ifndef LIBRARYPLUGINMENU_H
#define LIBRARYPLUGINMENU_H

#include <QMenu>
#include "Helper/Pimpl.h"

class LibraryPluginMenu :
		public QMenu
{
	Q_OBJECT
	PIMPL(LibraryPluginMenu)

public:
	LibraryPluginMenu(QWidget* parent=nullptr);
	~LibraryPluginMenu();

public slots:
	void setup_actions();

private slots:
	void action_triggered(bool b);
	void current_library_changed(const QString& name);
};

#endif // LIBRARYPLUGINMENU_H
