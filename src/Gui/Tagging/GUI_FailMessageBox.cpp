/* GUI_FailMessageBox.cpp */

/* Copyright (C) 2011-2020 Lucio Carreras
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

#include "GUI_FailMessageBox.h"
#include "Gui/TagEdit/ui_GUI_FailMessageBox.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Utils/Language/Language.h"
#include <QVBoxLayout>

GUI_FailMessageBox::GUI_FailMessageBox(QWidget* parent) :
	Gui::Dialog(parent)
{
	ui = new Ui::GUI_FailMessageBox();
	ui->setupUi(this);
	ui->tv_files->setVisible(false);
	ui->tv_files->setItemDelegate(new Gui::StyledItemDelegate());

	connect(ui->cb_details, &QCheckBox::toggled, this, &GUI_FailMessageBox::details_toggled);
	connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_FailMessageBox::close);

	language_changed();
}

GUI_FailMessageBox::~GUI_FailMessageBox()
{
	delete ui; ui=nullptr;
}

void GUI_FailMessageBox::set_failed_files(const QMap<QString, Tagging::Editor::FailReason>& files)
{
	ui->tv_files->clear();

	using Reason=Tagging::Editor::FailReason;
	ui->tv_files->setRowCount(files.size());
	ui->tv_files->setColumnCount(4);
	ui->tv_files->setHorizontalHeaderLabels(QStringList{
		Lang::get(Lang::Filename),
		tr("File exists") + "?",
		tr("Writeable") + "?",
		Lang::get(Lang::Error)
	});

	const QList<QString> keys = files.keys();
	int row=0;
	for(const QString& key : keys)
	{
		Reason reason = files[key];

		auto twi_filename = new QTableWidgetItem(key);
		twi_filename->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

		auto twi_exists = new QTableWidgetItem();
		twi_exists->setTextAlignment(Qt::AlignCenter);

		auto twi_writable = new QTableWidgetItem();
		twi_writable->setTextAlignment(Qt::AlignCenter);

		auto twi_error = new QTableWidgetItem();
		twi_writable->setTextAlignment(Qt::AlignCenter);
		twi_error->setText(QString::number(static_cast<int>(reason)));

		if(reason == Reason::FileNotFound)
		{
			twi_exists->setText(Lang::get(Lang::No));
			twi_writable->setText(Lang::get(Lang::No));
		}

		else if(reason == Reason::FileNotWriteable)
		{
			twi_exists->setText(Lang::get(Lang::Yes));
			twi_writable->setText(Lang::get(Lang::No));
		}

		else
		{
			twi_exists->setText(Lang::get(Lang::Yes));
			twi_writable->setText(Lang::get(Lang::Yes));
		}

		ui->tv_files->setItem(row, 0, twi_filename);
		ui->tv_files->setItem(row, 1, twi_exists);
		ui->tv_files->setItem(row, 2, twi_writable);
		ui->tv_files->setItem(row, 3, twi_error);

		ui->tv_files->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

		row++;
	}
}

void GUI_FailMessageBox::details_toggled(bool b)
{
	ui->tv_files->setVisible(b);

	if(b) {
		this->adjustSize();
		QSize sz = this->size();
		sz.setWidth(parentWidget()->width());
		this->resize(sz);
	}

	else {
		this->adjustSize();
	}
}

void GUI_FailMessageBox::language_changed()
{
	ui->lab_warning->setText(tr("Some files could not be saved"));
	ui->lab_header->setText(Lang::get(Lang::Warning));
}

void GUI_FailMessageBox::showEvent(QShowEvent* e)
{
	Gui::Dialog::showEvent(e);
	this->adjustSize();
}
