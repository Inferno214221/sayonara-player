/* GUI_Target_Playlist_Dialog.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#ifndef GUI_TARGET_PLAYLIST_DIALOG_H
#define GUI_TARGET_PLAYLIST_DIALOG_H

#include "Gui/Utils/Widgets/Dialog.h"
UI_FWD(GUI_TargetPlaylistDialog)

class GUI_TargetPlaylistDialog :
		public Gui::Dialog
{
	Q_OBJECT
	UI_CLASS(GUI_TargetPlaylistDialog)

public:
	explicit GUI_TargetPlaylistDialog(QWidget* parent=nullptr);
	~GUI_TargetPlaylistDialog() override;

signals:
	void sigTargetChosen(const QString& name, bool relative);

private slots:
	void searchButtonClicked();
	void okButtonClicked();

protected:
	void languageChanged() override;
};

#endif // GUI_TARGET_PLAYLIST_DIALOG_H
