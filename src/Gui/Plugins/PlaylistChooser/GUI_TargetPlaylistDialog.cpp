/* GUI_TargetPlaylistDialog.cpp */

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

#include "GUI_TargetPlaylistDialog.h"
#include "Gui/Plugins/ui_GUI_TargetPlaylistDialog.h"

#include "Utils/Language/Language.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include <QFileDialog>

GUI_TargetPlaylistDialog::GUI_TargetPlaylistDialog(QWidget* parent) :
	Dialog(parent)
{
	ui = new Ui::GUI_TargetPlaylistDialog();
	ui->setupUi(this);

	connect(ui->btnChoose, &QPushButton::clicked, this, &GUI_TargetPlaylistDialog::searchButtonClicked);
	connect(ui->btnOk, &QPushButton::clicked, this, &GUI_TargetPlaylistDialog::okButtonClicked);
}

GUI_TargetPlaylistDialog::~GUI_TargetPlaylistDialog()
{
	delete ui;
}

void GUI_TargetPlaylistDialog::languageChanged()
{
	ui->retranslateUi(this);
}

void GUI_TargetPlaylistDialog::searchButtonClicked()
{
	QString filename = QFileDialog::getSaveFileName(this,
	                                                Lang::get(Lang::SaveAs), QDir::homePath(), "*.m3u"
	);

	if(!filename.endsWith("m3u", Qt::CaseInsensitive))
	{
		filename.append(".m3u");
	}

	ui->lePath->setText(filename);
}

void GUI_TargetPlaylistDialog::okButtonClicked()
{
	QString targetPath = ui->lePath->text();
	bool checked = ui->cbRelative->isChecked();

	if(!targetPath.isEmpty())
	{
		emit sigTargetChosen(targetPath, checked);
		close();
	}
}
