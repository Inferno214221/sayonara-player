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
#include "Gui/Library/ui_GUI_EmptyLibrary.h"
#include "Gui/Utils/Library/GUI_EditLibrary.h"

#include "Components/Library/LibraryManager.h"
#include "Components/Library/LocalLibrary.h"
#include "Interfaces/LibraryInterface/LibraryPluginHandler.h"

#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/Library/LibraryNamespaces.h"

#include <QDir>
#include <QFileDialog>
#include <QComboBox>

using namespace Library;

struct GUI_EmptyLibrary::Private
{
	GUI_EditLibrary* new_library=nullptr;
};

GUI_EmptyLibrary::GUI_EmptyLibrary(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::GUI_EmptyLibrary();
	ui->setupUi(this);

	ui->pb_progress->setVisible(false);

	connect(ui->btn_setLibrary, &QPushButton::clicked,
			this, &GUI_EmptyLibrary::set_lib_path_clicked);
}

GUI_EmptyLibrary::~GUI_EmptyLibrary() {}

QFrame* GUI_EmptyLibrary::header_frame() const
{
	return ui->header_frame;
}

void GUI_EmptyLibrary::set_lib_path_clicked()
{
	if(!m->new_library)
	{
		m->new_library = new GUI_EditLibrary(this);
		connect(m->new_library, &GUI_EditLibrary::sig_accepted, this, &GUI_EmptyLibrary::new_library_created);
	}

	m->new_library->reset();
	m->new_library->show();
}

void GUI_EmptyLibrary::new_library_created()
{
	GUI_EditLibrary* new_library = dynamic_cast<GUI_EditLibrary*>(sender());
	if(!new_library) {
		return;
	}

	QString name = new_library->name();
	QString path = new_library->path();

	Manager* lib_manager = Manager::instance();

	LibraryId id = lib_manager->add_library(name, path);
	if(id < 0){
		return;
	}

	Message::Answer answer = Message::question_yn(tr("Do you want to reload the Library?"), "Library");

	if(answer == Message::Answer::No){
		return;
	}

	LocalLibrary* library = lib_manager->library_instance(id);
	library->reload_library(false, Library::ReloadQuality::Accurate);
}


void Library::GUI_EmptyLibrary::language_changed()
{
	if(ui)
	{
		ui->retranslateUi(this);
	}
}
