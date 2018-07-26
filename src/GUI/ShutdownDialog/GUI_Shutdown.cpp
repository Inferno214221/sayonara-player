/* GUI_Shutdown.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "GUI/ShutdownDialog/ui_GUI_Shutdown.h"

#include "GUI/Utils/Icons.h"

#include "Components/Shutdown/Shutdown.h"

#ifdef WITH_SHUTDOWN

GUI_Shutdown::GUI_Shutdown(QWidget* parent):
	Gui::Dialog(parent)
{
	ui = new Ui::GUI_Shutdown();
	ui->setupUi(this);

	connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_Shutdown::ok_clicked);
	connect(ui->btn_cancel, &QPushButton::clicked, this, &GUI_Shutdown::cancel_clicked);
	connect(ui->rb_after_finished, &QRadioButton::clicked, this, &GUI_Shutdown::rb_after_finished_clicked);
	connect(ui->rb_after_minutes, &QRadioButton::clicked, this, &GUI_Shutdown::rb_after_minutes_clicked);
}

GUI_Shutdown::~GUI_Shutdown()
{
	delete ui; ui=nullptr;
}

void GUI_Shutdown::skin_changed()
{
	ui->lab_icon->setPixmap(Gui::Icons::pixmap(Gui::Icons::Shutdown));
}


void GUI_Shutdown::ok_clicked()
{
	if(ui->sb_minutes->isEnabled())
	{
		MilliSeconds msec = ui->sb_minutes->value() * 60 * 1000;
		Shutdown::instance()->shutdown(msec);
	}

	else {
		Shutdown::instance()->shutdown_after_end();
	}

	close();
}

void GUI_Shutdown::cancel_clicked()
{
	close();
	emit sig_closed();
}

void GUI_Shutdown::rb_after_finished_clicked(bool b)
{
	Q_UNUSED(b)
	ui->rb_after_minutes->setChecked(false);
	ui->sb_minutes->setEnabled(false);
}

void GUI_Shutdown::rb_after_minutes_clicked(bool b)
{
	Q_UNUSED(b)
	ui->rb_after_minutes->setChecked(false);
	ui->sb_minutes->setEnabled(true);
}

#endif // WITH_SHUTDOWN
