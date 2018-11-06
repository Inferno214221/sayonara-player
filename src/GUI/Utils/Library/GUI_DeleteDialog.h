/* LibraryDeleteDialog.h */

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

#ifndef LIBRARYDELETEDIALOG_H
#define LIBRARYDELETEDIALOG_H

#include "Utils/Pimpl.h"
#include "Utils/Library/LibraryNamespaces.h"
#include "GUI/Utils/Widgets/Dialog.h"

#include <QMessageBox>

UI_FWD(GUI_DeleteDialog)

class GUI_DeleteDialog :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_DeleteDialog)
	UI_CLASS(GUI_DeleteDialog)

public:
	GUI_DeleteDialog(int n_tracks, QWidget* parent=nullptr);
	virtual ~GUI_DeleteDialog();

	void set_num_tracks(int n_tracks);

	Library::TrackDeletionMode answer() const;

private slots:
	void yes_clicked();
	void only_from_library_clicked();
	void no_clicked();

protected:
	void showEvent(QShowEvent* e);
};


#endif // LIBRARYDELETEDIALOG_H
