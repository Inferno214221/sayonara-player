/* DirectoryContextMenu.h */

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



#ifndef DIRECTORYCONTEXTMENU_H
#define DIRECTORYCONTEXTMENU_H

#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"

class DirectoryContextMenu :
		public Gui::LibraryContextMenu
{
	Q_OBJECT
	PIMPL(DirectoryContextMenu)

signals:
	void sig_create_dir_clicked();
	void sig_rename_clicked();
	void sig_collapse_all_clicked();

public:
	enum Mode
	{
		Dir=0,
		File
	};

	DirectoryContextMenu(Mode mode, QWidget* parent);
	~DirectoryContextMenu();

	void set_create_dir_visible(bool b);
	void set_rename_visible(bool b);
	void set_collapse_all_visibled(bool b);

protected:
	void language_changed() override;
	void skin_changed() override;
};

#endif // DIRECTORYCONTEXTMENU_H
