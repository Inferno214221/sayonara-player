/* LocalLibraryContainer.cpp */

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

#include "LocalLibraryContainer.h"
#include "Gui/Library/GUI_LocalLibrary.h"
#include "Gui/Utils/Icons.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Utils/Library/LibraryInfo.h"

#include <QAction>
#include <QIcon>

using namespace Library;

struct LocalLibraryContainer::Private
{
	Library::Manager* libraryManager;
	GUI_LocalLibrary* ui = nullptr;
	Info library;
	QString name;

	Private(Library::Manager* libraryManager, const Info& library) :
		libraryManager(libraryManager),
		library(library),
		name(library.name()) {}
};

LocalLibraryContainer::LocalLibraryContainer(Library::Manager* libraryManager, const Library::Info& library,
                                             Library::PluginHandler* pluginHandler) :
	Container(pluginHandler)
{
	m = Pimpl::make<Private>(libraryManager, library);
}

LocalLibraryContainer::~LocalLibraryContainer() = default;

QString LocalLibraryContainer::name() const
{
	QString name = displayName();
	name = name.toLower();
	name.replace(" ", "-");
	return name;
}

QString LocalLibraryContainer::displayName() const
{
	return m->name;
}

QWidget* LocalLibraryContainer::widget() const
{
	return static_cast<QWidget*>(m->ui);
}

QMenu* LocalLibraryContainer::menu()
{
	if(m->ui)
	{
		return m->ui->menu();
	}

	return nullptr;
}

void LocalLibraryContainer::initUi()
{
	if(!m->ui)
	{
		m->ui = new GUI_LocalLibrary(m->library.id(), m->libraryManager);
	}
}

bool LocalLibraryContainer::isLocal() const
{
	return true;
}

QFrame* LocalLibraryContainer::header() const
{
	return m->ui->headerFrame();
}

QIcon LocalLibraryContainer::icon() const
{
	return Gui::Icons::icon(Gui::Icons::LocalLibrary);
}

void LocalLibraryContainer::rename(const QString& name)
{
	m->name = name;
}
