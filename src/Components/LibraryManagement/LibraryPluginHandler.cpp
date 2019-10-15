/* LibraryPluginHandler.cpp */

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
	Container*			empty_library=nullptr;
	Container*			current_library=nullptr;
	ContainerList		library_containers;

	Container* find_library(const QString& name)
	{
		sp_log(Log::Debug, this) << "Searching for Library " << name;

		auto it = Algorithm::find(library_containers, [&name](Container* c){
			return (c->name() == name);
		});

		if (it != library_containers.end())
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
	m->empty_library = fallback_library;

	QString last_library = GetSetting(Set::Lib_CurPlugin);
	init_libraries(containers);
	init_dll_libraries();

	{ // First startup handling
		bool has_local_library = Util::Algorithm::contains(m->library_containers, [](Container* container){
			return container->is_local();
		});

		Container* c = m->find_library(last_library);
		if(!has_local_library && (c == nullptr))
		{
			set_current_library(-1);
		}

		else
		{
			set_current_library(last_library);
		}
	}
}


void PluginHandler::init_libraries(const QList<Library::Container*>& containers)
{
	for(Container* container : containers)
	{
		if(!container) {
			continue;
		}

		sp_log(Log::Debug, this) << "Add library " << container->display_name();

		m->library_containers << container;
	}
}

void PluginHandler::init_dll_libraries()
{
	QDir plugin_dir = QDir(Util::lib_path());
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
				sp_log(Log::Debug, this) << "Have found valid plugin with iid = " << value;
				has_correct_iid = true;
			}

			else if(value.startsWith("com.sayonara-player."))
			{
				sp_log(Log::Info, this) << "Ignoring *outdated* plugin with iid = " << value << " at " << absolute_path;
				sp_log(Log::Info, this) << "You can delete this file";
			}

			else
			{
				sp_log(Log::Warning, this) << "Ignoring *invalid* plugin with iid = " << value << " at " << absolute_path;
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
			sp_log(Log::Warning, this) << "Cannot load plugin: " << filename << ": " << loader.errorString();
			loader.unload();
			continue;
		}

		Container* container = dynamic_cast<Container*>(raw_plugin);
		if(!container)
		{
			loader.unload();
			continue;
		}

		sp_log(Log::Info, this) << "Found library plugin " << container->display_name();

		m->library_containers << container;
	}
}

void PluginHandler::set_current_library(const QString& name)
{
	set_current_library( m->find_library(name) );
}

void PluginHandler::set_current_library(int index)
{
	Container* ret = m->empty_library;

	if(!m->library_containers.isEmpty() && index >= 0)
	{
		index = std::min(index, m->library_containers.size() - 1);
		ret = m->library_containers[index];
	}

	set_current_library(ret);
}

void PluginHandler::set_current_library(Container* cur_library)
{
	if(!cur_library)
	{
		if(m->library_containers.isEmpty())
		{
			cur_library	= m->empty_library;
		}

		else {
			cur_library = m->library_containers.first();
		}
	}

	m->current_library = cur_library;

	if(m->current_library)
	{
		m->current_library->init();
	}

	SetSetting(Set::Lib_CurPlugin, cur_library->name() );

	emit sig_current_library_changed();
}

Container* PluginHandler::current_library() const
{
	return m->current_library;
}

QWidget* PluginHandler::current_library_widget() const
{
	if(!m->current_library){
		return nullptr;
	}

	return m->current_library->widget();
}

void PluginHandler::add_local_library(Library::Container* container)
{
	if(container == nullptr) {
		return;
	}

	int idx = Algorithm::indexOf(m->library_containers, [=](Container* c){
		return (c->is_local() == false && c != m->empty_library);
	});

	idx = std::max(idx, 0);
	m->library_containers.insert(idx, container);

	emit sig_libraries_changed();

	set_current_library(idx);
}

void PluginHandler::rename_local_library(const QString& old_name, const QString& new_name)
{
	Container* c = m->find_library(convert_display_name(old_name));
	if(c && c->is_local())
	{
		c->rename(new_name);
		emit sig_libraries_changed();
	}
}

void PluginHandler::remove_local_library(const QString& name)
{
	Container* c = m->find_library(convert_display_name(name));
	if(c && c->is_local())
	{
		m->library_containers.removeAll(c);

		if(m->current_library == c)
		{
			set_current_library(0);
		}

		emit sig_libraries_changed();
	}
}

void PluginHandler::move_local_library(int old_index, int new_index)
{
	// first index is empty library
	if( !Util::between(old_index, m->library_containers) ||
		!Util::between(new_index, m->library_containers))
	{
		return;
	}

	m->library_containers.move(old_index, new_index);

	emit sig_libraries_changed();
}

QList<Library::Container*> PluginHandler::get_libraries(bool also_empty) const
{
	QList<Container*> containers = m->library_containers;
	if(also_empty) {
		containers.push_front(m->empty_library);
	}

	return containers;
}
