/* GUI_ReloadLibraryDialog.h */

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



#ifndef GUI_RELOADLIBRARYDIALOG_H
#define GUI_RELOADLIBRARYDIALOG_H

#include "Gui/Utils/Widgets/Dialog.h"

#include "Utils/Pimpl.h"
#include "Utils/Library/LibraryNamespaces.h"

UI_FWD(GUI_ReloadLibraryDialog)

class GUI_ReloadLibraryDialog :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_ReloadLibraryDialog)
	UI_CLASS(GUI_ReloadLibraryDialog)

signals:
	void sig_accepted(Library::ReloadQuality quality);

public:
	explicit GUI_ReloadLibraryDialog(const QString& library_name, QWidget *parent=nullptr);
	~GUI_ReloadLibraryDialog();

	void set_quality(Library::ReloadQuality quality);

private slots:
	void ok_clicked();
	void cancel_clicked();
	void combo_changed(int);

protected:
	void language_changed() override;
};

#endif // GUI_RELOADLIBRARYDIALOG_H
