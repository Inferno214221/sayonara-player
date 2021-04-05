/* GUI_Shutdown.cpp */

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

#include "GUI_Shutdown.h"
#include "Gui/Shutdown/ui_GUI_Shutdown.h"

#include "Utils/Macros.h"
#include "Gui/Utils/Icons.h"

#include "Components/Shutdown/Shutdown.h"

#ifdef SAYONARA_WITH_SHUTDOWN

GUI_Shutdown::GUI_Shutdown(QWidget* parent):
	Gui::Dialog(parent)
{
	ui = new Ui::GUI_Shutdown();
	ui->setupUi(this);

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GUI_Shutdown::accepted);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GUI_Shutdown::rejected);
	connect(ui->rb_after_finished, &QRadioButton::clicked, this, &GUI_Shutdown::afterPlaylistFinishedClicked);
	connect(ui->rb_after_minutes, &QRadioButton::clicked, this, &GUI_Shutdown::afterTimespanClicked);
}

GUI_Shutdown::~GUI_Shutdown()
{
	delete ui; ui=nullptr;
}

void GUI_Shutdown::skinChanged()
{
	ui->lab_icon->setPixmap(Gui::Icons::pixmap(Gui::Icons::Shutdown, ui->lab_icon->size()));
}


void GUI_Shutdown::accepted()
{
	if(ui->sb_minutes->isEnabled())
	{
		MilliSeconds msec = ui->sb_minutes->value() * 60 * 1000;
		Shutdown::instance()->shutdown(msec);
	}

	else {
		Shutdown::instance()->shutdownAfterSessionEnd();
	}

	close();
}

void GUI_Shutdown::rejected()
{
	close();
	emit sigClosed();
}

void GUI_Shutdown::afterPlaylistFinishedClicked(bool b)
{
	Q_UNUSED(b)
	ui->rb_after_minutes->setChecked(false);
	ui->sb_minutes->setEnabled(false);
}

void GUI_Shutdown::afterTimespanClicked(bool b)
{
	Q_UNUSED(b)
	ui->rb_after_minutes->setChecked(false);
	ui->sb_minutes->setEnabled(true);
}

#endif // SAYONARA_WITH_SHUTDOWN
