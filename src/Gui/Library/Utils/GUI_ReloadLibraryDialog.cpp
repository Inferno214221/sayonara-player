/* GUI_ReloadLibraryDialog.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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
	QString library_name;

	Private(const QString& library_name) :
		library_name(library_name)
	{}
};

GUI_LibraryReloadDialog::GUI_LibraryReloadDialog(const QString& library_name, QWidget *parent) :
	Gui::Dialog(parent),
	ui(new Ui::GUI_LibraryReloadDialog)
{
	m = Pimpl::make<Private>(library_name);

	ui->setupUi(this);

	this->setModal(true);

	connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_LibraryReloadDialog::ok_clicked);
	connect(ui->btn_cancel, &QPushButton::clicked, this, &GUI_LibraryReloadDialog::cancel_clicked);
	connect(ui->combo_quality, combo_activated_int, this, &GUI_LibraryReloadDialog::combo_changed);
}


GUI_LibraryReloadDialog::~GUI_LibraryReloadDialog()
{
	delete ui;
}

void GUI_LibraryReloadDialog::set_quality(Library::ReloadQuality quality)
{
	switch(quality)
	{
		case Library::ReloadQuality::Accurate:
			ui->combo_quality->setCurrentIndex(1);
			break;
		default:
			ui->combo_quality->setCurrentIndex(0);
	}
}


void GUI_LibraryReloadDialog::language_changed()
{
	ui->btn_ok->setText(Lang::get(Lang::OK));
	ui->btn_cancel->setText(Lang::get(Lang::Cancel));
	ui->lab_title->setText(Lang::get(Lang::ReloadLibrary) + ": " + m->library_name);

	ui->combo_quality->clear();
	ui->combo_quality->addItem(tr("Fast scan"));
	ui->combo_quality->addItem(tr("Deep scan"));

	combo_changed(ui->combo_quality->currentIndex());

	this->setWindowTitle(Lang::get(Lang::ReloadLibrary) + ": " + m->library_name);
}

void GUI_LibraryReloadDialog::ok_clicked()
{
	int cur_idx = ui->combo_quality->currentIndex();
	if(cur_idx == 0)
	{
		emit sig_accepted(Library::ReloadQuality::Fast);
	}

	else if(cur_idx == 1)
	{
		emit sig_accepted(Library::ReloadQuality::Accurate);
	}

	close();
}

void GUI_LibraryReloadDialog::cancel_clicked()
{
	ui->combo_quality->setCurrentIndex(0);

	close();
}

void GUI_LibraryReloadDialog::combo_changed(int i)
{
	if(i == 0){
		ui->lab_description->setText(tr("Only scan for new and deleted files"));
	}

	else{
		ui->lab_description->setText(tr("Scan all files in your library directory"));
	}
}
