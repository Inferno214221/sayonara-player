/* LibraryDeleteDialog.cpp */

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

#include "GUI_DeleteDialog.h"
#include "Utils/Language/Language.h"
#include "Gui/ui_GUI_DeleteDialog.h"
#include "Gui/Utils/Icons.h"


struct GUI_DeleteDialog::Private
{
	int	n_tracks;
	Library::TrackDeletionMode answer;

	Private(int n_tracks) :
		n_tracks(n_tracks),
		answer(Library::TrackDeletionMode::None)
	{}
};


GUI_DeleteDialog::GUI_DeleteDialog(int n_tracks, QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>(n_tracks);

	ui = new Ui::GUI_DeleteDialog();
	ui->setupUi(this);

	connect(ui->btn_yes, &QPushButton::clicked, this, &GUI_DeleteDialog::yes_clicked);
	connect(ui->btn_no, &QPushButton::clicked, this, &GUI_DeleteDialog::no_clicked);
	connect(ui->btn_only_from_library, &QPushButton::clicked, this, &GUI_DeleteDialog::only_from_library_clicked);
}

GUI_DeleteDialog::~GUI_DeleteDialog() {}


Library::TrackDeletionMode GUI_DeleteDialog::answer() const
{
	return m->answer;
}

void GUI_DeleteDialog::yes_clicked()
{
	m->answer = Library::TrackDeletionMode::AlsoFiles;
	close();
}

void GUI_DeleteDialog::only_from_library_clicked()
{
	m->answer = Library::TrackDeletionMode::OnlyLibrary;
	close();
}

void GUI_DeleteDialog::no_clicked()
{
	m->answer = Library::TrackDeletionMode::None;
	close();
}

void GUI_DeleteDialog::showEvent(QShowEvent* e)
{
	Gui::Dialog::showEvent(e);

	this->setFocus();

	ui->lab_icon->setPixmap(Gui::Icons::pixmap(Gui::Icons::Info));
	ui->btn_yes->setText(Lang::get(Lang::OK));
	ui->btn_no->setText(Lang::get(Lang::Cancel));
	ui->btn_only_from_library->setText(tr("Only from library"));
	ui->lab_warning->setText(Lang::get(Lang::Warning) + "!");
	ui->lab_info->setText(
			tr("You are about to delete %1 files").arg(m->n_tracks) +
				"\n" +
				Lang::get(Lang::Continue).question());
}

void GUI_DeleteDialog::set_num_tracks(int n_tracks)
{
	m->n_tracks = n_tracks;
}

