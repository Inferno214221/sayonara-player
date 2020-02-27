/* LocalLibraryWatcher.cpp */

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

#include "LocalLibraryWatcher.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Components/Library/LocalLibrary.h"
#include "Gui/Library/LocalLibraryContainer.h"

#include "Utils/Library/LibraryInfo.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <QMap>

using namespace Library;

struct LocalLibraryWatcher::Private
{
	QList<Info> infos;

	Private()
	{
		infos = Manager::instance()->allLibraries();
	}
};

LocalLibraryWatcher::LocalLibraryWatcher(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	Manager* manager = Manager::instance();
	connect(manager, &Manager::sigAdded, this, &LocalLibraryWatcher::libraryAdded);
	connect(manager, &Manager::sigMoved, this, &LocalLibraryWatcher::libraryMoved);
	connect(manager, &Manager::sigRenamed, this, &LocalLibraryWatcher::libraryRenamed);
	connect(manager, &Manager::sigRemoved, this, &LocalLibraryWatcher::libraryRemoved);
}

LocalLibraryWatcher::~LocalLibraryWatcher() = default;

QList<AbstractContainer*> LocalLibraryWatcher::getLocalLibraryContainers() const
{
	QList<AbstractContainer*> containers;

	for(const Info& info : m->infos)
	{
		containers << new LocalLibraryContainer(info);
	}

	return containers;
}


void LocalLibraryWatcher::libraryAdded(LibraryId id)
{
	Manager* manager = Manager::instance();
	Info info = manager->libraryInfo(id);
	m->infos = manager->allLibraries();

	AbstractContainer* c = new LocalLibraryContainer(info);
	PluginHandler* lph = PluginHandler::instance();
	lph->addLocalLibrary(c);
}

void LocalLibraryWatcher::libraryMoved(LibraryId id, int from, int to)
{
	Q_UNUSED(id)

	PluginHandler::instance()->moveLocalLibrary(from, to);

	m->infos = Manager::instance()->allLibraries();
}

void LocalLibraryWatcher::libraryRenamed(LibraryId id)
{
	Manager* manager = Manager::instance();
	Info info = manager->libraryInfo(id);
	int idx = Util::Algorithm::indexOf(m->infos, [id](const Info& info){
		return (info.id() == id);
	});

	if(Util::between(idx, m->infos))
	{
		QString old_name = m->infos[idx].name();
		QString new_name = info.name();

		PluginHandler::instance()->renameLocalLibrary(old_name, new_name);
	}

	m->infos = manager->allLibraries();
}

void LocalLibraryWatcher::libraryRemoved(LibraryId id)
{
	PluginHandler* lph = PluginHandler::instance();
	int idx = Util::Algorithm::indexOf(m->infos, [id](const Info& info){
		return (info.id() == id);
	});

	if(Util::between(idx, m->infos))
	{
		lph->removeLocalLibrary(m->infos[idx].name());
	}

	m->infos = Manager::instance()->allLibraries();
}
