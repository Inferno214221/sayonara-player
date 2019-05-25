/* GUI_EditLibrary.cpp */

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

#include "GUI_EditLibrary.h"
#include "Gui/Utils/ui_GUI_EditLibrary.h"
#include "Utils/Language.h"
#include "Utils/FileUtils.h"

#include <QFileDialog>
#include <QSizePolicy>
#include <QStringList>

struct GUI_EditLibrary::Private
{
	QString old_name;
	QString old_path;

	EditMode edit_mode;
	bool name_edited;

	Private() :
		edit_mode(EditMode::New),
		name_edited(false)
	{}
};

GUI_EditLibrary::GUI_EditLibrary(QWidget *parent) :
	Dialog (parent),
	ui(new Ui::GUI_EditLibrary)
{
	ui->setupUi(this);

	m = Pimpl::make<Private>();
	m->edit_mode = EditMode::New;

	ui->btn_choose_dir->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	ui->le_path->setFocus();

	connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_EditLibrary::ok_clicked);
	connect(ui->btn_cancel, &QPushButton::clicked, this, &GUI_EditLibrary::cancel_clicked);
	connect(ui->btn_choose_dir, &QPushButton::clicked, this, &GUI_EditLibrary::choose_dir_clicked);
	connect(ui->le_name, &QLineEdit::textEdited, this, &GUI_EditLibrary::name_edited);
}

GUI_EditLibrary::GUI_EditLibrary(const QString& name, const QString& path, QWidget* parent) :
	GUI_EditLibrary(parent)
{
	m->edit_mode = EditMode::Edit;
	m->name_edited = true;

	m->old_name = name;
	m->old_path = path;

	ui->le_name->setText(name);
	ui->le_path->setText(path);
	ui->lab_title->setText(Lang::get(Lang::Edit));

	this->setWindowTitle(ui->lab_title->text());
	this->setAttribute(Qt::WA_DeleteOnClose);
}


GUI_EditLibrary::~GUI_EditLibrary()
{
	delete ui; ui = nullptr;
}

void GUI_EditLibrary::ok_clicked()
{
	close();
	emit sig_accepted();
}

void GUI_EditLibrary::cancel_clicked()
{
	ui->le_path->clear();
	ui->le_name->clear();
	close();

	emit sig_recected();
}

void GUI_EditLibrary::choose_dir_clicked()
{
	QString old_dir = m->old_path;
	if(old_dir.isEmpty()){
		old_dir = QDir::homePath();
	}

	QString new_dir = QFileDialog::getExistingDirectory(this,
														Lang::get(Lang::Directory),
														old_dir,
														QFileDialog::ShowDirsOnly);

	if(new_dir.isEmpty()) {
		new_dir = m->old_path;
	}

	if(m->edit_mode == EditMode::New)
	{
		QString str = Util::File::get_filename_of_path(new_dir);

		if(!m->name_edited)
		{
			ui->le_name->setText(str);
		}
	}

	ui->le_path->setText(new_dir);
}

void GUI_EditLibrary::name_edited(const QString& text)
{
	m->name_edited = (text.size() > 0);
}

QString GUI_EditLibrary::name() const
{
	return ui->le_name->text();
}

QString GUI_EditLibrary::path() const
{
	return ui->le_path->text();
}

bool GUI_EditLibrary::has_name_changed() const
{
	return (name() != m->old_name);
}

bool GUI_EditLibrary::has_path_changed() const
{
	return (path() != m->old_path);
}

GUI_EditLibrary::EditMode GUI_EditLibrary::edit_mode() const
{
	return m->edit_mode;
}

void GUI_EditLibrary::reset()
{
	ui->le_name->setText(QString());
	ui->le_path->setText(QString());

	m->old_name = QString();
	m->old_path = QString();
	m->edit_mode = EditMode::New;
	m->name_edited = false;
}

void GUI_EditLibrary::language_changed()
{
	Dialog::language_changed();

	ui->btn_ok->setText(Lang::get(Lang::OK));
	ui->btn_cancel->setText(Lang::get(Lang::Cancel));
	ui->lab_path->setText(Lang::get(Lang::Directory));
	ui->lab_name->setText(Lang::get(Lang::Name));

	if(m->edit_mode == EditMode::New) {
		ui->lab_title->setText(Lang::get(Lang::New));
	} else {
		ui->lab_title->setText(Lang::get(Lang::Edit));
	}

	this->setWindowTitle(ui->lab_title->text());
}

void GUI_EditLibrary::skin_changed()
{
	Dialog::skin_changed();
}
