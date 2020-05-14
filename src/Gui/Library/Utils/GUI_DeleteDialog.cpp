/* LibraryDeleteDialog.cpp */

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

#include "GUI_DeleteDialog.h"
#include "Utils/Language/Language.h"
#include "Gui/Utils/ui_GUI_DeleteDialog.h"
#include "Gui/Utils/Icons.h"

#include <QPushButton>

struct GUI_DeleteDialog::Private
{
	int	trackCount;
	Library::TrackDeletionMode answer;

	Private(int trackCount) :
		trackCount(trackCount),
		answer(Library::TrackDeletionMode::None)
	{}
};


GUI_DeleteDialog::GUI_DeleteDialog(int trackCount, QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>(trackCount);

	ui = new Ui::GUI_DeleteDialog();
	ui->setupUi(this);

	connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &GUI_DeleteDialog::buttonClicked);
}

GUI_DeleteDialog::~GUI_DeleteDialog() = default;


Library::TrackDeletionMode GUI_DeleteDialog::answer() const
{
	return m->answer;
}

void GUI_DeleteDialog::buttonClicked(QAbstractButton* button)
{
	QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(button);
	if(role == QDialogButtonBox::YesRole)
	{
		if(ui->cbOnlyFromLibrary->isChecked())
		{
			m->answer = Library::TrackDeletionMode::OnlyLibrary;
		}

		else
		{
			m->answer = Library::TrackDeletionMode::AlsoFiles;
		}
	}

	else if(role == QDialogButtonBox::NoRole)
	{
		m->answer = Library::TrackDeletionMode::None;
	}

	close();
}

void GUI_DeleteDialog::showEvent(QShowEvent* e)
{
	Gui::Dialog::showEvent(e);

	this->setFocus();

	ui->labIcon->setPixmap(Gui::Icons::pixmap(Gui::Icons::Info));
	ui->cbOnlyFromLibrary->setText(tr("Only from library"));
	ui->labWarning->setText(Lang::get(Lang::Warning) + "!");
	ui->labInfo->setText
	(
		tr("You are about to delete %n file(s)", "", m->trackCount) +
			"\n" +
			Lang::get(Lang::Continue).question()
	);
}

void GUI_DeleteDialog::setTrackCount(int trackCount)
{
	m->trackCount = trackCount;
}

