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
	Library::Manager* libraryManager;
	Library::PluginHandler* libraryPluginHandler;
	QList<Info> infos;

	Private(Library::Manager* libraryManager, Library::PluginHandler* pluginHandler) :
		libraryManager {libraryManager},
		libraryPluginHandler {pluginHandler},
		infos {libraryManager->allLibraries()} {}
};

LocalLibraryWatcher::LocalLibraryWatcher(Library::Manager* libraryManager, Library::PluginHandler* pluginHandler,
                                         QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(libraryManager, pluginHandler);

	connect(libraryManager, &Manager::sigAdded, this, &LocalLibraryWatcher::libraryAdded);
	connect(libraryManager, &Manager::sigMoved, this, &LocalLibraryWatcher::libraryMoved);
	connect(libraryManager, &Manager::sigRenamed, this, &LocalLibraryWatcher::libraryRenamed);
	connect(libraryManager, &Manager::sigRemoved, this, &LocalLibraryWatcher::libraryRemoved);
}

LocalLibraryWatcher::~LocalLibraryWatcher() = default;

QList<LibraryContainer*> LocalLibraryWatcher::getLocalLibraryContainers() const
{
	QList<LibraryContainer*> containers;
	Util::Algorithm::transform(m->infos, containers, [this](const Info& info) {
		return new LocalLibraryContainer(m->libraryManager, info, m->libraryPluginHandler);
	});

	return containers;
}

void LocalLibraryWatcher::libraryAdded(LibraryId id)
{
	const auto info = m->libraryManager->libraryInfo(id);
	m->infos = m->libraryManager->allLibraries();

	auto* container = new LocalLibraryContainer(m->libraryManager, info, m->libraryPluginHandler);
	m->libraryPluginHandler->addLocalLibrary(container);
}

void LocalLibraryWatcher::libraryMoved(LibraryId id, int from, int to)
{
	Q_UNUSED(id)

	m->libraryPluginHandler->moveLocalLibrary(from, to);

	m->infos = m->libraryManager->allLibraries();
}

void LocalLibraryWatcher::libraryRenamed(LibraryId id)
{
	const auto info = m->libraryManager->libraryInfo(id);
	const auto idx = Util::Algorithm::indexOf(m->infos, [id](const Info& info) {
		return (info.id() == id);
	});

	if(Util::between(idx, m->infos))
	{
		const auto oldName = m->infos[idx].name();
		const auto newName = info.name();

		m->libraryPluginHandler->renameLocalLibrary(oldName, newName);
	}

	m->infos = m->libraryManager->allLibraries();
}

void LocalLibraryWatcher::libraryRemoved(LibraryId id)
{
	const auto idx = Util::Algorithm::indexOf(m->infos, [id](const Info& info) {
		return (info.id() == id);
	});

	if(Util::between(idx, m->infos))
	{
		m->libraryPluginHandler->removeLocalLibrary(m->infos[idx].name());
	}

	m->infos = m->libraryManager->allLibraries();
}
