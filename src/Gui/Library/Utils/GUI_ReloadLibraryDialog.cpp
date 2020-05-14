/* GUI_ReloadLibraryDialog.cpp */

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

#include "GUI_ReloadLibraryDialog.h"
#include "Gui/Library/ui_GUI_ReloadLibraryDialog.h"

#include "Utils/Language/Language.h"
#include <QComboBox>

using Library::GUI_LibraryReloadDialog;
struct GUI_LibraryReloadDialog::Private
{
	QString libraryName;

	Private(const QString& library_name) :
		libraryName(library_name)
	{}
};

GUI_LibraryReloadDialog::GUI_LibraryReloadDialog(const QString& library_name, QWidget* parent) :
	Gui::Dialog(parent),
	ui(new Ui::GUI_LibraryReloadDialog)
{
	m = Pimpl::make<Private>(library_name);

	ui->setupUi(this);

	this->setModal(true);

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GUI_LibraryReloadDialog::okClicked);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GUI_LibraryReloadDialog::cancelClicked);
	connect(ui->comboQuality, combo_activated_int, this, &GUI_LibraryReloadDialog::comboChanged);
}

GUI_LibraryReloadDialog::~GUI_LibraryReloadDialog()
{
	delete ui;
}

void GUI_LibraryReloadDialog::setQuality(Library::ReloadQuality quality)
{
	switch(quality)
	{
		case Library::ReloadQuality::Accurate:
			ui->comboQuality->setCurrentIndex(1);
			break;
		default:
			ui->comboQuality->setCurrentIndex(0);
	}
}

void GUI_LibraryReloadDialog::languageChanged()
{
	ui->labTitle->setText(Lang::get(Lang::ReloadLibrary) + ": " + m->libraryName);

	ui->comboQuality->clear();
	ui->comboQuality->addItem(tr("Fast scan"));
	ui->comboQuality->addItem(tr("Deep scan"));

	comboChanged(ui->comboQuality->currentIndex());

	this->setWindowTitle(Lang::get(Lang::ReloadLibrary) + ": " + m->libraryName);
}

void GUI_LibraryReloadDialog::okClicked()
{
	int currentIndex = ui->comboQuality->currentIndex();
	if(currentIndex == 0)
	{
		emit sigAccepted(Library::ReloadQuality::Fast);
	}

	else if(currentIndex == 1)
	{
		emit sigAccepted(Library::ReloadQuality::Accurate);
	}

	close();
}

void GUI_LibraryReloadDialog::cancelClicked()
{
	ui->comboQuality->setCurrentIndex(0);

	close();
}

void GUI_LibraryReloadDialog::comboChanged(int i)
{
	if(i == 0){
		ui->labDescription->setText(tr("Only scan for new and deleted files"));
	}

	else{
		ui->labDescription->setText(tr("Scan all files in your library directory"));
	}
}
