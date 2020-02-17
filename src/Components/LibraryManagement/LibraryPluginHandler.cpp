/* LibraryPluginHandler.cpp */

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

#include "LibraryPluginHandler.h"
#include "Interfaces/Library/LibraryContainer.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QPluginLoader>
#include <QJsonObject>
#include <QVariantMap>

using Library::PluginHandler;
using Library::Container;

using ContainerList=QList<Container*>;

namespace Algorithm=Util::Algorithm;

struct PluginHandler::Private
{
	Container*			emptyLibrary=nullptr;
	Container*			currentLibrary=nullptr;
	ContainerList		libraryContainers;

	Container* find_library(const QString& name)
	{
		spLog(Log::Debug, this) << "Searching for Library " << name;

		auto it = Algorithm::find(libraryContainers, [&name](Container* c){
			return (c->name() == name);
		});

		if (it != libraryContainers.end())
		{
			return *it;
		}

		return nullptr;
	}
};

static QString convert_display_name(const QString& display_name)
{
	QString ret = display_name.toLower().trimmed();
	ret.replace(" ", "-");

	return ret;
}


/*************************************************************************/

PluginHandler::PluginHandler() :
	QObject(nullptr)
{
	m = Pimpl::make<Private>();
}

PluginHandler::~PluginHandler() = default;

void PluginHandler::init(const ContainerList& containers, Container* fallback_library)
{
	m->emptyLibrary = fallback_library;

	QString last_library = GetSetting(Set::Lib_CurPlugin);
	initLibraries(containers);
	initDllLibraries();

	{ // First startup handling
		bool has_local_library = Util::Algorithm::contains(m->libraryContainers, [](Container* container){
			return container->isLocal();
		});

		Container* c = m->find_library(last_library);
		if(!has_local_library && (c == nullptr))
		{
			setCurrentLibrary(-1);
		}

		else
		{
			setCurrentLibrary(last_library);
		}
	}
}


void PluginHandler::initLibraries(const QList<Library::Container*>& containers)
{
	for(Container* container : containers)
	{
		if(!container) {
			continue;
		}

		spLog(Log::Debug, this) << "Add library " << container->displayName();

		m->libraryContainers << container;
	}
}

void PluginHandler::initDllLibraries()
{
	QDir plugin_dir = QDir(Util::libPath());
	QStringList dll_filenames = plugin_dir.entryList(QDir::Files);

	for(const QString& filename : dll_filenames)
	{
		QString absolute_path = plugin_dir.absoluteFilePath(filename);
		QPluginLoader loader(absolute_path);
		QJsonObject metadata = loader.metaData();
		QVariantMap map = metadata.toVariantMap();
		const auto keys = map.keys();

		bool has_correct_iid = false;
		for(const QString& key : keys)
		{
			if(key.trimmed() != "IID"){
				continue;
			}

			QString value = map[key].toString();

			if(value.startsWith("com.sayonara-player.apiv2."))
			{
				spLog(Log::Debug, this) << "Have found valid plugin with iid = " << value;
				has_correct_iid = true;
			}

			else if(value.startsWith("com.sayonara-player."))
			{
				spLog(Log::Info, this) << "Ignoring *outdated* plugin with iid = " << value << " at " << absolute_path;
				spLog(Log::Info, this) << "You can delete this file";
			}

			else
			{
				spLog(Log::Warning, this) << "Ignoring *invalid* plugin with iid = " << value << " at " << absolute_path;
			}

			break;
		}

		if(!has_correct_iid)
		{
			continue;
		}

		QObject* raw_plugin = loader.instance();
		if(!raw_plugin)
		{
			spLog(Log::Warning, this) << "Cannot load plugin: " << filename << ": " << loader.errorString();
			loader.unload();
			continue;
		}

		Container* container = dynamic_cast<Container*>(raw_plugin);
		if(!container)
		{
			loader.unload();
			continue;
		}

		spLog(Log::Info, this) << "Found library plugin " << container->displayName();

		m->libraryContainers << container;
	}
}

void PluginHandler::setCurrentLibrary(const QString& name)
{
	setCurrentLibrary( m->find_library(name) );
}

void PluginHandler::setCurrentLibrary(int index)
{
	Container* ret = m->emptyLibrary;

	if(!m->libraryContainers.isEmpty() && index >= 0)
	{
		index = std::min(index, m->libraryContainers.size() - 1);
		ret = m->libraryContainers[index];
	}

	setCurrentLibrary(ret);
}

void PluginHandler::setCurrentLibrary(Container* cur_library)
{
	if(!cur_library)
	{
		if(m->libraryContainers.isEmpty())
		{
			cur_library	= m->emptyLibrary;
		}

		else {
			cur_library = m->libraryContainers.first();
		}
	}

	m->currentLibrary = cur_library;

	if(m->currentLibrary)
	{
		m->currentLibrary->init();
	}

	SetSetting(Set::Lib_CurPlugin, cur_library->name());

	emit sigCurrentLibraryChanged();
}

Container* PluginHandler::currentLibrary() const
{
	return m->currentLibrary;
}

QWidget* PluginHandler::currentLibraryWidget() const
{
	if(!m->currentLibrary){
		return nullptr;
	}

	return m->currentLibrary->widget();
}

void PluginHandler::addLocalLibrary(Library::Container* container)
{
	if(container == nullptr) {
		return;
	}

	int idx = Algorithm::indexOf(m->libraryContainers, [=](Container* c){
		return (c->isLocal() == false && c != m->emptyLibrary);
	});

	idx = std::max(idx, 0);
	m->libraryContainers.insert(idx, container);

	emit sigLibrariesChanged();

	setCurrentLibrary(idx);
}

void PluginHandler::renameLocalLibrary(const QString& old_name, const QString& new_name)
{
	Container* c = m->find_library(convert_display_name(old_name));
	if(c && c->isLocal())
	{
		c->rename(new_name);
		emit sigLibrariesChanged();
	}
}

void PluginHandler::removeLocalLibrary(const QString& name)
{
	Container* c = m->find_library(convert_display_name(name));
	if(c && c->isLocal())
	{
		m->libraryContainers.removeAll(c);

		if(m->currentLibrary == c)
		{
			setCurrentLibrary(0);
		}

		emit sigLibrariesChanged();
	}
}

void PluginHandler::moveLocalLibrary(int old_index, int new_index)
{
	// first index is empty library
	if( !Util::between(old_index, m->libraryContainers) ||
		!Util::between(new_index, m->libraryContainers))
	{
		return;
	}

	m->libraryContainers.move(old_index, new_index);

	emit sigLibrariesChanged();
}

QList<Library::Container*> PluginHandler::libraries(bool also_empty) const
{
	QList<Container*> containers = m->libraryContainers;
	if(also_empty) {
		containers.push_front(m->emptyLibrary);
	}

	return containers;
}
