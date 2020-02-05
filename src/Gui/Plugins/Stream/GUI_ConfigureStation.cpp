/* GUI_ConfigureStation.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "GUI_ConfigureStation.h"
#include "Gui/Plugins/ui_GUI_ConfigureStation.h"
#include "Utils/Language/Language.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

struct GUI_ConfigureStation::Private
{
	QList<QLabel*> labels;
};

GUI_ConfigureStation::GUI_ConfigureStation(QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::GUI_ConfigureStation();
	ui->setupUi(this);
	ui->lab_error->setVisible(false);
	ui->btn_ok->setFocus();

	connect(ui->btn_ok, &QPushButton::clicked, this, &Gui::Dialog::accept);
	connect(ui->btn_cancel, &QPushButton::clicked, this, &Gui::Dialog::reject);
}

GUI_ConfigureStation::~GUI_ConfigureStation() = default;

void GUI_ConfigureStation::init_ui(StationPtr station)
{
	const QList<QWidget*> config_widgets = configuration_widgets(station);

	auto* layout = dynamic_cast<QGridLayout*>(ui->config_widget->layout());

	int row = 0;
	for(QWidget* config_widget : config_widgets)
	{
		auto* label = new QLabel();
		label->setText(label_text(row));
		m->labels << label;

		layout->addWidget(label, row, 0, 1, 1);
		layout->addWidget(config_widget, row, 1, 1, 1);
		row++;
	}
}

void GUI_ConfigureStation::set_error_message(const QString& message)
{
	ui->lab_error->setText(message);
	ui->lab_error->setVisible(true);
}

void GUI_ConfigureStation::set_mode(const QString& type, GUI_ConfigureStation::Mode mode)
{
	QString mode_str;
	if(mode == GUI_ConfigureStation::Edit){
		mode_str = Lang::get(Lang::Edit);
	}

	else {
		mode_str = Lang::get(Lang::New);
	}

	QString text = QString("%1: %2").arg(type).arg(mode_str);

	ui->lab_header->setText(text);
	this->setWindowTitle(text);
}

bool GUI_ConfigureStation::was_accepted() const
{
	return (this->result() == QDialog::Accepted);
}
