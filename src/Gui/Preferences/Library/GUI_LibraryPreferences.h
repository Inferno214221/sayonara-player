/* GUI_LibraryPreferences.h */

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

#ifndef GUI_LIBRARYPREFERENCES_H
#define GUI_LIBRARYPREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_LibraryPreferences)

namespace Library
{
	class Manager;
}

class GUI_LibraryPreferences :
	public Preferences::Base
{
	Q_OBJECT
	PIMPL(GUI_LibraryPreferences)
	UI_CLASS(GUI_LibraryPreferences)

	public:
		GUI_LibraryPreferences(Library::Manager* libraryManager, const QString& identifier);
		~GUI_LibraryPreferences() override;

		bool commit() override;
		void revert() override;

		QString actionName() const override;

	protected:
		void initUi() override;
		void retranslate() override;
		void skinChanged() override;

		void showEvent(QShowEvent* e) override;
		QString errorString() const override;

	private slots:
		void newClicked();
		void editClicked();
		void deleteClicked();

		void upClicked();
		void downClicked();

		void editDialogAccepted();
		void selectedIndexChanged(const QModelIndex& idx);

	private:
		int currentRow() const;
};

#endif // GUI_LIBRARYPREFERENCES_H
