/* GUI_FailMessageBox.h */

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



#ifndef FAILMESSAGEBOX_H
#define FAILMESSAGEBOX_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Components/Tagging/Editor.h"

#include <QMap>
#include <QString>

UI_FWD(GUI_FailMessageBox)

/**
 * @brief The GUI_FailMessageBox class
 * @ingroup GuiTagging
 */
class GUI_FailMessageBox : public Gui::Dialog
{
	Q_OBJECT
	UI_CLASS(GUI_FailMessageBox)

public:
	GUI_FailMessageBox(QWidget* parent=nullptr);
	~GUI_FailMessageBox() override;

	void setFailedFiles(const QMap<QString, Tagging::Editor::FailReason>& failed_files);

private slots:
	void detailsToggled(bool b);

protected:
	void languageChanged() override;
	void showEvent(QShowEvent* e) override;
};

#endif // FAILMESSAGEBOX_H
