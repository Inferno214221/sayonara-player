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

#include "LocalLibraryObserver.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Gui/Library/LocalLibraryContainer.h"
#include "Utils/Algorithm.h"
#include "Utils/Library/LibraryInfo.h"

namespace Library
{
	struct LocalLibraryObserver::Private
	{
		Library::Manager* libraryManager;
		Library::PluginHandler* libraryPluginHandler;
		QList<Info> infos;

		Private(Library::Manager* libraryManager, Library::PluginHandler* pluginHandler) :
			libraryManager {libraryManager},
			libraryPluginHandler {pluginHandler},
			infos {libraryManager->allLibraries()} {}
	};

	LocalLibraryObserver::LocalLibraryObserver(Library::Manager* libraryManager, Library::PluginHandler* pluginHandler,
	                                           QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>(libraryManager, pluginHandler)}
	{
		connect(libraryManager, &Manager::sigAdded, this, &LocalLibraryObserver::libraryAdded);
		connect(libraryManager, &Manager::sigMoved, this, &LocalLibraryObserver::libraryMoved);
		connect(libraryManager, &Manager::sigRenamed, this, &LocalLibraryObserver::libraryRenamed);
		connect(libraryManager, &Manager::sigRemoved, this, &LocalLibraryObserver::libraryRemoved);
	}

	LocalLibraryObserver::~LocalLibraryObserver() = default;

	QList<LibraryContainer*> LocalLibraryObserver::getLocalLibraryContainers() const
	{
		QList<LibraryContainer*> containers;
		Util::Algorithm::transform(m->infos, containers, [this](const Info& info) {
			return new LocalLibraryContainer(m->libraryManager, info, m->libraryPluginHandler);
		});

		return containers;
	}

	void LocalLibraryObserver::libraryAdded(const LibraryId id)
	{
		const auto info = m->libraryManager->libraryInfo(id);
		m->infos = m->libraryManager->allLibraries();

		auto* container = new LocalLibraryContainer(m->libraryManager, info, m->libraryPluginHandler);
		m->libraryPluginHandler->addLocalLibrary(container);
	}

	void LocalLibraryObserver::libraryMoved(const LibraryId /*id*/, const int from, const int to)
	{
		m->libraryPluginHandler->moveLocalLibrary(from, to);
		m->infos = m->libraryManager->allLibraries();
	}

	void LocalLibraryObserver::libraryRenamed(const LibraryId id)
	{
		const auto it = Util::Algorithm::find(m->infos, [id](const auto& info) {
			return (info.id() == id);
		});

		if(it != m->infos.end())
		{
			const auto oldName = it->name();
			const auto newName = m->libraryManager->libraryInfo(id).name();

			m->libraryPluginHandler->renameLocalLibrary(oldName, newName);
		}

		m->infos = m->libraryManager->allLibraries();
	}

	void LocalLibraryObserver::libraryRemoved(const LibraryId id)
	{
		const auto it = Util::Algorithm::find(m->infos, [id](const auto& info) {
			return (info.id() == id);
		});

		if(it != m->infos.end())
		{
			m->libraryPluginHandler->removeLocalLibrary(it->name());
		}

		m->infos = m->libraryManager->allLibraries();
	}
}