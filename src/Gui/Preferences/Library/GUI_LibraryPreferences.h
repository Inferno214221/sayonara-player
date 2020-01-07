/* GUI_LibraryPreferences.h */

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

#ifndef GUI_LIBRARYPREFERENCES_H
#define GUI_LIBRARYPREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_LibraryPreferences)

class GUI_LibraryPreferences :
		public Preferences::Base
{
	Q_OBJECT
	PIMPL(GUI_LibraryPreferences)
	UI_CLASS(GUI_LibraryPreferences)

public:
	explicit GUI_LibraryPreferences(const QString& identifier);
	~GUI_LibraryPreferences() override;

	bool commit() override;
	void revert() override;

	QString action_name() const override;

protected:
	void init_ui() override;
	void retranslate_ui() override;
	void skin_changed() override;

	void showEvent(QShowEvent* e) override;
	QString error_string() const override;

private slots:
	void new_clicked();
	void edit_clicked();
	void delete_clicked();

	void up_clicked();
	void down_clicked();

	void edit_dialog_accepted();
	void current_index_changed(const QModelIndex& idx);

private:
	int current_row() const;
};



#endif // GUI_LIBRARYPREFERENCES_H
