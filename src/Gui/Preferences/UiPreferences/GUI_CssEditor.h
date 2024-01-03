/* GUI_CssEditor.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef GUI_CSSEDITOR_H
#define GUI_CSSEDITOR_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_CssEditor)

class GUI_CssEditor : public
	Gui::Dialog
{
		Q_OBJECT
		PIMPL(GUI_CssEditor)
		UI_CLASS(GUI_CssEditor)

	public:
		explicit GUI_CssEditor(QWidget* parent = nullptr);
		~GUI_CssEditor() override;

	private slots:
		void saveClicked();
		void applyClicked();
		void undoClicked();
		void darkModeToggled(bool b);

	protected:
		void showEvent(QShowEvent* e) override;
		void skinChanged() override;
};

#endif // GUI_CSSEDITOR_H
