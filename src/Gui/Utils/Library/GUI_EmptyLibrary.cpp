/* GUI_EmptyLibrary.cpp */

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

#include "GUI_EmptyLibrary.h"
#include "Gui/Utils/ui_GUI_EmptyLibrary.h"
#include "Gui/Utils/Library/GUI_EditLibrary.h"
#include "Components/Library/LibraryManager.h"

#include "Utils/Library/LibraryInfo.h"
#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"
#include "Utils/Algorithm.h"

#include <QFileDialog>

using namespace Library;

GUI_EmptyLibrary::GUI_EmptyLibrary(QWidget* parent) :
	Gui::Widget(parent)
{
	ui = new Ui::GUI_EmptyLibrary();
	ui->setupUi(this);

	bool ok = check_name() && check_path();

	ui->btn_ok->setEnabled(ok);
	ui->lab_error->setVisible(ok);

	connect(ui->le_name, &QLineEdit::textChanged, this, &GUI_EmptyLibrary::name_changed);
	connect(ui->le_path, &QLineEdit::textChanged, this, &GUI_EmptyLibrary::path_changed);
	connect(ui->btn_choose_dir, &QPushButton::clicked, this, &GUI_EmptyLibrary::choose_dir_clicked);
	connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_EmptyLibrary::ok_clicked);
}

GUI_EmptyLibrary::~GUI_EmptyLibrary() {}

QFrame* GUI_EmptyLibrary::header_frame() const
{
	return ui->header_frame;
}

void GUI_EmptyLibrary::ok_clicked()
{
	QString name = ui->le_name->text();
	QString path = ui->le_path->text();

	Manager::instance()->add_library(name, path);
}


void GUI_EmptyLibrary::choose_dir_clicked()
{
	static QString old_dir = QDir::homePath();

	QString new_dir = QFileDialog::getExistingDirectory(this,
						Lang::get(Lang::Directory),
						old_dir,
						QFileDialog::ShowDirsOnly);

	if(new_dir.isEmpty()){
		return;
	}

	old_dir = new_dir;
	ui->le_path->setText(new_dir);
}

bool GUI_EmptyLibrary::check_name()
{
	Manager* manager = Manager::instance();
	QString name = ui->le_name->text();

	if(name.isEmpty()) {
		ui->lab_error->setText(tr("Please choose a name for your library"));
		return false;
	}

	QList<Library::Info> infos = manager->all_libraries();
	bool contains = Util::Algorithm::contains(infos, [&name](const Library::Info& info){
		return (name.toLower() == info.name().toLower());
	});

	if(contains)
	{
		ui->lab_error->setText(tr("Please choose another name for your library"));
		return false;
	}

	return true;
}

void GUI_EmptyLibrary::name_changed(const QString& str)
{
	Q_UNUSED(str)

	bool ok = check_path() && check_name();

	ui->btn_ok->setEnabled(ok);
	ui->lab_error->setVisible(!ok);
}

bool GUI_EmptyLibrary::check_path()
{
	Manager* manager = Manager::instance();
	QString path = ui->le_path->text();

	if(!Util::File::exists(path))
	{
		ui->lab_error->setText(tr("The file path is invalid"));
		return false;
	}

	if(manager->library_info_by_path(path).valid())
	{
		ui->lab_error->setText(tr("A library with the same file path already exists"));
		return false;
	}

	return true;
}

void GUI_EmptyLibrary::path_changed(const QString& str)
{
	Q_UNUSED(str)

	Manager* manager = Manager::instance();
	QString path = ui->le_path->text();
	QString name = manager->request_library_name(path);
	ui->le_name->setText(name);

	bool ok = check_path() && check_name();

	ui->btn_ok->setEnabled(ok);
	ui->lab_error->setVisible(!ok);
}


void Library::GUI_EmptyLibrary::language_changed()
{
	ui->retranslateUi(this);
	ui->lab_path->setText(Lang::get(Lang::Directory));
	ui->lab_name->setText(Lang::get(Lang::Name));
	ui->btn_ok->setText(Lang::get(Lang::OK) + "!");
	ui->lab_title->setText(Lang::get(Lang::CreateNewLibrary));
}
