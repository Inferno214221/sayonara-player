/* GUI_FileExpressionDialog.h
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

#ifndef GUI_FILEEXPRESSIONDIALOG_H
#define GUI_FILEEXPRESSIONDIALOG_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

class GUI_FileExpressionDialog :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_FileExpressionDialog)

public:
	GUI_FileExpressionDialog(QWidget* parent=nullptr);
	~GUI_FileExpressionDialog() override;

	QString expression() const;

protected:
	void showEvent(QShowEvent* event) override;
	void languageChanged() override;

private slots:
	void buttonClicked();
	void textChanged(const QString& text);
};

#endif // GUI_FILEEXPRESSIONDIALOG_H
