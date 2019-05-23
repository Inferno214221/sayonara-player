/* EmptyLibraryContainer.cpp */

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

#include "EmptyLibraryContainer.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Library/GUI_EmptyLibrary.h"
#include "Gui/Library/EmptyLibraryContainer.h"

#include <QIcon>
#include <QComboBox>
#include <QPixmap>

using namespace Library;

struct EmptyLibraryContainer::Private
{
	GUI_EmptyLibrary* ui=nullptr;

};

EmptyLibraryContainer::EmptyLibraryContainer(QObject* parent) :
	Library::Container(parent)
{
	m = Pimpl::make<Private>();
}

EmptyLibraryContainer::~EmptyLibraryContainer() {}

QString EmptyLibraryContainer::name() const
{
	return "empty-library";
}

QString EmptyLibraryContainer::display_name() const
{
	return tr("Empty Library");
}

QWidget* EmptyLibraryContainer::widget() const
{
	return static_cast<QWidget*>(m->ui);
}

QMenu*EmptyLibraryContainer::menu()
{
	return nullptr;
}

void EmptyLibraryContainer::init_ui()
{
	m->ui = new GUI_EmptyLibrary();
}


QFrame* EmptyLibraryContainer::header() const
{
	return m->ui->header_frame();
}

QPixmap EmptyLibraryContainer::icon() const
{
	return Gui::Icons::icon(Gui::Icons::New).pixmap(32, 32);
}
