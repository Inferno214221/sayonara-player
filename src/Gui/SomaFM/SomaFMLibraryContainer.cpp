/* SomaFMLibraryContainer.cpp */

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


/* SomaFMLibraryContainer.cpp */

#include "SomaFMLibraryContainer.h"
#include "Components/Streaming/SomaFM/SomaFMLibrary.h"
#include "GUI_SomaFM.h"

#include <QIcon>

static void soma_fm_init_icons()
{
	Q_INIT_RESOURCE(SomaFMIcons);
}

struct SomaFM::LibraryContainer::Private
{
	SomaFM::Library* library;

	Private(SomaFM::Library* library) :
		library(library)
	{}
};

SomaFM::LibraryContainer::LibraryContainer(SomaFM::Library* library, QObject* parent) :
	::Library::Container(parent)
{
	m = Pimpl::make<Private>(library);
	soma_fm_init_icons();
}

SomaFM::LibraryContainer::~LibraryContainer() = default;

QString SomaFM::LibraryContainer::name() const
{
	return "SomaFM";
}

QString SomaFM::LibraryContainer::displayName() const
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

void SomaFM::LibraryContainer::initUi()
{
	ui = new GUI_SomaFM(m->library, nullptr);
}

QIcon SomaFM::LibraryContainer::icon() const
{
	return QIcon(":/soma_icons/soma.png");
}

QFrame* SomaFM::LibraryContainer::header() const
{
	return ui->headerFrame();
}
