/* GUI_EditLibrary.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef GUI_EDITLIBRARY_H
#define GUI_EDITLIBRARY_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/Dialog.h"

UI_FWD(GUI_EditLibrary)

class GUI_EditLibrary :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_EditLibrary)
	UI_CLASS_SHARED_PTR(GUI_EditLibrary)

	signals:
		void sigAccepted();
		void sigRejected();

	public:
		explicit GUI_EditLibrary(const QString& name, const QString& path, QWidget* parent = nullptr);
		explicit GUI_EditLibrary(QWidget* parent = nullptr);
		~GUI_EditLibrary() override;

		void reset();

		enum class EditMode
		{
			New = 0,
			Edit = 1
		};

		QString name() const;
		QString path() const;

		bool hasNameChanged() const;
		bool hasPathChanged() const;

		EditMode editMode() const;

	private slots:
		void okClicked();
		void cancelClicked();
		void chooseDirClicked();
		void nameEdited(const QString& text);

	protected:
		void languageChanged() override;
};

#endif // GUI_EDITLIBRARY_H
