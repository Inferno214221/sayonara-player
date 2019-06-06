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
#include "LibraryContainer/LibraryContainer.h"

#include "Private/LibraryPluginCombobox.h"
#include "Gui/Utils/Library/EmptyLibraryContainer.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QFrame>
#include <QLayout>
#include <QMenu>
#include <QPluginLoader>
#include <QVBoxLayout>

using Library::PluginHandler;
using Library::Container;
using Library::PluginCombobox;

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

	bool has_empty_library() const
	{
		if(library_containers.isEmpty()){
			return false;
		}

		return (library_containers.first() == empty_library);
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

void PluginHandler::init(const ContainerList& containers)
{
	m->empty_library = new EmptyLibraryContainer(this);

	QString last_plugin = GetSetting(Set::Lib_CurPlugin);
	init_libraries(containers);
	init_dll_libraries();

	Container* container = m->find_library(last_plugin);
	if(!container)
	{
		container = m->library_containers.first();
	}

	set_current_library(container);
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

	check_local_library();
}

void PluginHandler::init_dll_libraries()
{
	QDir plugin_dir = QDir(Util::lib_path());
	QStringList dll_filenames = plugin_dir.entryList(QDir::Files);

	for(const QString& filename : dll_filenames)
	{
		QPluginLoader loader(plugin_dir.absoluteFilePath(filename));

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


void PluginHandler::init_library(Container* library)
{
	if(library->is_initialized()) {
		return;
	}

	library->init_ui();
	library->set_initialized();

	QWidget* ui = library->widget();
	QLayout* layout = ui->layout();
	if(layout) {
		layout->setContentsMargins(5, 0, 8, 0);
	}

	QFrame* header_frame = library->header();
	if(header_frame)
	{
		PluginCombobox* combo_box = new PluginCombobox(library->display_name(), nullptr);

		QLayout* layout = new QVBoxLayout(header_frame);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(combo_box);

		header_frame->setFrameShape(QFrame::NoFrame);
		header_frame->setLayout(layout);

		connect(combo_box, combo_activated_int, this, &PluginHandler::current_library_changed);
	}
}

bool PluginHandler::check_local_library()
{
	bool has_local = Algorithm::contains(m->library_containers, [](Container* c){
		return c->is_local();
	});

	if(!has_local)
	{
		if(!m->has_empty_library())
		{
			m->library_containers.push_front(m->empty_library);

			return true;
		}
	}

	else
	{
		if(m->has_empty_library())
		{
			m->library_containers.takeFirst();

			if(m->current_library == m->empty_library)
			{
				this->set_current_library(m->library_containers.first());
			}

			return true;
		}
	}

	return false;
}

void PluginHandler::current_library_changed(int library_idx)
{
	if(Util::between(library_idx, m->library_containers))
	{
		Container* c = m->library_containers[library_idx];
		set_current_library(c);
	}
}

void PluginHandler::set_current_library(const QString& name)
{
	set_current_library( m->find_library(name) );
}

void PluginHandler::set_current_library(Container* cur_library)
{
	if(!cur_library) {
		cur_library = m->library_containers.first();
	}

	if(m->current_library)
	{
		m->current_library->hide();
	}

	m->current_library = cur_library;

	if(m->current_library)
	{
	   init_library(m->current_library);
	}

	SetSetting(Set::Lib_CurPlugin, cur_library->name() );

	emit sig_current_library_changed( cur_library->name() );
}

Container* PluginHandler::current_library() const
{
	return m->current_library;
}

QMenu* PluginHandler::current_library_menu() const
{
	if(!m->current_library) {
		return nullptr;
	}

	return m->current_library->menu();
}

void PluginHandler::add_local_library(Library::Container* container)
{
	if(container == nullptr)
	{
		return;
	}

	int idx = 1;
	if(!m->has_empty_library())
	{
		idx = Algorithm::indexOf(m->library_containers, [](Container* c){
			return (c->is_local() == false);
		});
	}

	m->library_containers.insert(std::max(idx, 0), container);
	check_local_library();

	emit sig_libraries_changed();

	set_current_library(container);
}

void PluginHandler::rename_local_library(const QString& old_name, const QString& new_name)
{
	Container* c = m->find_library(convert_display_name(old_name));
	if(c && c->is_local())
	{
		c->set_name(new_name);
		emit sig_libraries_changed();
	}
}

void PluginHandler::remove_local_library(const QString& name)
{
	Container* c = m->find_library(convert_display_name(name));
	if(c && c->is_local())
	{
		c->hide();
		m->library_containers.removeAll(c);
		check_local_library();

		if(m->current_library == c)
		{
			set_current_library(m->library_containers.first());
		}

		emit sig_libraries_changed();
	}
}

void PluginHandler::move_local_library(int old_index, int new_index)
{
	if( m->has_empty_library() ||
		!Util::between(old_index, m->library_containers) ||
		!Util::between(new_index, m->library_containers))
	{
		return;
	}

	m->library_containers.move(old_index, new_index);

	emit sig_libraries_changed();
}


QList<Library::Container*> PluginHandler::get_libraries() const
{
	return m->library_containers;
}
