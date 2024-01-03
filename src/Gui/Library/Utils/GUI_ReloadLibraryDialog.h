/* GUI_ReloadLibraryDialog.h */

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

#ifndef GUI_LIBRARYRELOAD_DIALOG_H
#define GUI_LIBRARYRELOAD_DIALOG_H

#include "Gui/Utils/Widgets/Dialog.h"

#include "Utils/Pimpl.h"
#include "Utils/Library/LibraryNamespaces.h"

UI_FWD(GUI_LibraryReloadDialog)

namespace Library
{
	/**
	 * @brief Reload dialog containing a combo box which allows you
	 * choosing between fast and deep reloading
	 * @ingroup GuiLibrary
	 */
	class GUI_LibraryReloadDialog :
		public Gui::Dialog
	{
		Q_OBJECT
		PIMPL(GUI_LibraryReloadDialog)
		UI_CLASS(GUI_LibraryReloadDialog)

		signals:
			void sigAccepted(ReloadQuality quality);

		public:
			explicit GUI_LibraryReloadDialog(const QString& libraryName, QWidget* parent = nullptr);
			~GUI_LibraryReloadDialog() override;

			void setQuality(ReloadQuality quality);

		private slots:
			void okClicked();
			void cancelClicked();
			void comboChanged(int);

		protected:
			void languageChanged() override;
	};
}

#endif // GUI_LIBRARYRELOAD_DIALOG_H
