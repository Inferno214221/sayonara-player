/* GUI_ConfigureStation.cpp */

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

#include "GUI_ConfigureStation.h"
#include "Gui/Plugins/ui_GUI_ConfigureStation.h"
#include "Utils/Language/Language.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

struct GUI_ConfigureStation::Private
{
	QList<QLabel*>				labels;
	GUI_ConfigureStation::Mode	mode;
	bool						isInitialized;

	Private() :
		mode(GUI_ConfigureStation::Mode::New),
		isInitialized(false)
	{}
};

GUI_ConfigureStation::GUI_ConfigureStation(QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::GUI_ConfigureStation();

	ui->setupUi(this);
	ui->labError->setVisible(false);

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &Gui::Dialog::accept);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &Gui::Dialog::reject);
}

GUI_ConfigureStation::~GUI_ConfigureStation() = default;

void GUI_ConfigureStation::init_ui()
{
	if(m->isInitialized){
		return;
	}

	m->isInitialized = true;

	const QList<QWidget*> configWidgets = configurationWidgets();

	auto* layout = dynamic_cast<QGridLayout*>(ui->configWidget->layout());

	int row = 0;
	for(QWidget* configWidget : configWidgets)
	{
		auto* label = new QLabel();
		label->setText(labelText(row));
		m->labels << label;

		layout->addWidget(label, row, 0, 1, 1);
		layout->addWidget(configWidget, row, 1, 1, 1);

		row++;
	}
}

void GUI_ConfigureStation::setError(const QString& message)
{
	ui->labError->setText(message);
	ui->labError->setVisible(true);
}

void GUI_ConfigureStation::setMode(const QString& stream_name, GUI_ConfigureStation::Mode mode)
{
	m->mode = mode;

	QString modeString;
	if(mode == GUI_ConfigureStation::Edit) {
		modeString = Lang::get(Lang::Edit);
	}

	else if(mode == GUI_ConfigureStation::New) {
		modeString = Lang::get(Lang::New);
	}

	else if(mode == GUI_ConfigureStation::Save) {
		modeString = Lang::get(Lang::Save);
	}

	QString text = QString("%1: %2").arg(stream_name).arg(modeString);

	ui->labHeader->setText(text);
	this->setWindowTitle(text);
}

GUI_ConfigureStation::Mode GUI_ConfigureStation::mode() const
{
	return m->mode;
}

void GUI_ConfigureStation::languageChanged()
{
	for(int i=0; i<m->labels.size(); i++)
	{
		m->labels[i]->setText(labelText(i));
	}
}
