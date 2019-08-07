/* LocalLibraryWatcher.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
#include "Interfaces/Library/LibraryPluginHandler.h"
#include "Components/Library/LibraryManager.h"
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
		infos = Manager::instance()->all_libraries();
	}
};

LocalLibraryWatcher::LocalLibraryWatcher(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	Manager* manager = Manager::instance();
	connect(manager, &Manager::sig_added, this, &LocalLibraryWatcher::library_added);
	connect(manager, &Manager::sig_moved, this, &LocalLibraryWatcher::library_moved);
	connect(manager, &Manager::sig_renamed, this, &LocalLibraryWatcher::library_renamed);
	connect(manager, &Manager::sig_removed, this, &LocalLibraryWatcher::library_removed);
}

LocalLibraryWatcher::~LocalLibraryWatcher() = default;

QList<Container*> LocalLibraryWatcher::get_local_library_containers() const
{
	QList<Container*> containers;

	for(const Info& info : m->infos)
	{
		containers << new LocalLibraryContainer(info);
	}

	return containers;
}


void LocalLibraryWatcher::library_added(LibraryId id)
{
	Manager* manager = Manager::instance();
	Info info = manager->library_info(id);
	m->infos = manager->all_libraries();

	Container* c = new LocalLibraryContainer(info);
	PluginHandler* lph = PluginHandler::instance();
	lph->add_local_library(c);
}

void LocalLibraryWatcher::library_moved(LibraryId id, int from, int to)
{
	Q_UNUSED(id)

	PluginHandler::instance()->move_local_library(from, to);

	m->infos = Manager::instance()->all_libraries();
}

void LocalLibraryWatcher::library_renamed(LibraryId id)
{
	Manager* manager = Manager::instance();
	Info info = manager->library_info(id);
	int idx = Util::Algorithm::indexOf(m->infos, [id](const Info& info){
		return (info.id() == id);
	});

	if(Util::between(idx, m->infos))
	{
		QString old_name = m->infos[idx].name();
		QString new_name = info.name();

		PluginHandler::instance()->rename_local_library(old_name, new_name);
	}

	m->infos = manager->all_libraries();
}

void LocalLibraryWatcher::library_removed(LibraryId id)
{
	PluginHandler* lph = PluginHandler::instance();
	int idx = Util::Algorithm::indexOf(m->infos, [id](const Info& info){
		return (info.id() == id);
	});

	if(Util::between(idx, m->infos))
	{
		lph->remove_local_library(m->infos[idx].name());
	}

	m->infos = Manager::instance()->all_libraries();
}
