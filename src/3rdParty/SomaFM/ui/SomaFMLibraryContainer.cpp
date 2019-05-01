/* SomaFMLibraryContainer.cpp */

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


/* SomaFMLibraryContainer.cpp */

#include "ui/SomaFMLibraryContainer.h"
#include "ui/GUI_SomaFM.h"

static void soma_fm_init_icons()
{
	Q_INIT_RESOURCE(SomaFMIcons);
}

SomaFM::LibraryContainer::LibraryContainer(QObject* parent) :
	::Library::Container(parent)
{
	soma_fm_init_icons();
}

SomaFM::LibraryContainer::~LibraryContainer() {}

QString SomaFM::LibraryContainer::name() const
{
	return "SomaFM";
}

QString SomaFM::LibraryContainer::display_name() const
{
	return "SomaFM";
}

QWidget* SomaFM::LibraryContainer::widget() const
{
	return ui;
}

QMenu* SomaFM::LibraryContainer::menu()
{
	return nullptr;
}

void SomaFM::LibraryContainer::init_ui()
{
	ui = new GUI_SomaFM(nullptr);
}

QPixmap SomaFM::LibraryContainer::icon() const
{
	return QPixmap(":/soma_icons/soma.png");
}

QFrame* SomaFM::LibraryContainer::header() const
{
	return ui->header_frame();
}
