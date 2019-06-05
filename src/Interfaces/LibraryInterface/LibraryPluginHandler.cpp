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
#include "LibraryPluginCombobox.h"
#include "LibraryContainer/LibraryContainer.h"

#include "Components/Library/LibraryManager.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Library/LocalLibraryContainer.h"
#include "Gui/Library/EmptyLibraryContainer.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"

#include <QAction>
#include <QDir>
#include <QFrame>
#include <QIcon>
#include <QLayout>
#include <QMenu>
#include <QPluginLoader>
#include <QSize>
#include <QVBoxLayout>

using Library::PluginHandler;
using Library::Container;
using Library::Manager;
using Library::PluginCombobox;

using ContainerList=QList<Container*>;
using LocalContainerList=QList<LocalLibraryContainer*>;

namespace Algorithm=Util::Algorithm;

struct PluginHandler::Private
{
	Container*			current_library=nullptr;
	Container*			empty_library=nullptr;

	LocalContainerList	local_libraries;
	ContainerList		library_containers;
	ContainerList		dll_libraries;

	QWidget*			library_parent=nullptr;
	PluginHandler*		plugin_handler=nullptr;

	Private(PluginHandler* plugin_handler) :
		plugin_handler(plugin_handler)
	{}

	void insert_local_libraries()
	{
		using Library::Info;
		const QList<Info> library_infos = Manager::instance()->all_libraries();
		for(const Info& library_info : library_infos)
		{
			if(library_info.id() < 0) {
				continue;
			}

			sp_log(Log::Debug, plugin_handler) << "Add local library "
											   << library_info.name() << ": "
											   << library_info.path();

			local_libraries << new LocalLibraryContainer(library_info);
		}

		if(local_libraries.isEmpty()) {
			empty_library = new EmptyLibraryContainer();
		}
	}

	void insert_dll_libraries()
	{
		QDir plugin_dir = QDir(Util::lib_path());
		QStringList dll_filenames = plugin_dir.entryList(QDir::Files);

		for(const QString& filename : dll_filenames)
		{
			QPluginLoader loader(plugin_dir.absoluteFilePath(filename));

			QObject* raw_plugin = loader.instance();
			if(!raw_plugin)
			{
				sp_log(Log::Warning, plugin_handler) << "Cannot load plugin: " << filename << ": " << loader.errorString();
				loader.unload();
				continue;
			}

			Container* container = dynamic_cast<Container*>(raw_plugin);
			if(!container)
			{
				loader.unload();
				continue;
			}

			sp_log(Log::Info, plugin_handler) << "Found library plugin " << container->display_name();

			dll_libraries << container;
		}
	}

	void insert_containers(const ContainerList& containers)
	{
		for(Container* container : containers)
		{
			if(!container) {
				continue;
			}

			sp_log(Log::Debug, plugin_handler) << "Add plugin " << container->display_name();
			library_containers << container;
		}
	}

	ContainerList all_libraries() const
	{
		ContainerList containers;
		if(empty_library) {
			containers << empty_library;
		}

		for(LocalLibraryContainer* llc : local_libraries) {
			containers << static_cast<Container*>(llc);
		}

		containers << library_containers;
		containers << dll_libraries;

		return containers;
	}

	Container* library_by_name(const QString& name)
	{
		const ContainerList containers = all_libraries();

		auto it = Algorithm::find(containers, [&name](Container* c){
			return (c->name() == name);
		});

		if (it == containers.end()) {
			return nullptr;
		}

		return *it;
	}
};


/*************************************************************************/


PluginHandler::PluginHandler() :
	QObject(nullptr)
{
	m = Pimpl::make<Private>(this);
}

PluginHandler::~PluginHandler() {}


void PluginHandler::init(const ContainerList& containers)
{
	QString cur_plugin = GetSetting(Set::Lib_CurPlugin);

	m->insert_local_libraries();
	m->insert_containers(containers);
	m->insert_dll_libraries();

	Container* container = m->library_by_name(cur_plugin);
	if(container) {
		set_current_library(container);
	}

	else {
		set_current_library( m->all_libraries().first() );
	}

	Manager* manager = Manager::instance();
	connect(manager, &Manager::sig_added, this, &PluginHandler::local_library_added);
	connect(manager, &Manager::sig_renamed, this, &PluginHandler::local_library_renamed);
	connect(manager, &Manager::sig_removed, this, &PluginHandler::local_library_removed);
	connect(manager, &Manager::sig_moved, this, &PluginHandler::local_library_moved);
}


void PluginHandler::init_library(Container* library)
{
	if(library->is_initialized()) {
		return;
	}

	library->init_ui();
	library->set_initialized();

	QWidget* ui = library->widget();
	ui->setParent(m->library_parent);

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


void PluginHandler::current_library_changed(int library_idx)
{
	ContainerList libs = m->all_libraries();
	if(between(library_idx, libs))
	{
		set_current_library(libs[library_idx]);
	}
}


void PluginHandler::set_current_library(const QString& name)
{
	set_current_library( m->library_by_name(name) );
}


void PluginHandler::set_current_library(Container* cur_library)
{
	if(!cur_library) {
		return;
	}

	const QList<Library::Container*> containers = m->all_libraries();
	for(Container* container : containers)
	{
		QString name = container->name();
		QString cur_name = cur_library->name();
		if(name.compare(cur_name) != 0) {
			container->hide();
		}

		else {
			m->current_library = container;
		}
	}

	if(m->current_library)
	{
	   init_library(m->current_library);
	}

	SetSetting(Set::Lib_CurPlugin, cur_library->name() );

	emit sig_current_library_changed( cur_library->name() );
}

void PluginHandler::local_library_added(LibraryId id)
{
	Library::Info info = Manager::instance()->library_info(id);
	if(!info.valid()){
		return;
	}

	LocalLibraryContainer* llc = new LocalLibraryContainer(info);
	m->local_libraries << llc;

	bool empty_library_found = (m->empty_library != nullptr);

	if(empty_library_found)
	{
		m->empty_library->hide();
		m->empty_library->deleteLater();
		m->empty_library = nullptr;
	}

	emit sig_libraries_changed();

	if(empty_library_found) {
		set_current_library(llc);
	}
}

void PluginHandler::local_library_renamed(LibraryId id)
{
	for(LocalLibraryContainer* llc : Algorithm::AsConst(m->local_libraries))
	{
		if(llc->id() == id)
		{
			Library::Info info = Manager::instance()->library_info(id);
			if(info.valid())
			{
				llc->set_name(info.name());
				break;
			}
		}
	}

	emit sig_libraries_changed();
}

void PluginHandler::local_library_removed(LibraryId id)
{
	int idx = -1;
	int i=0;

	LocalLibraryContainer* removed_llc=nullptr;
	for(LocalLibraryContainer* llc : Algorithm::AsConst(m->local_libraries))
	{
		if(llc->id() == id)
		{
			idx = i;
			removed_llc = llc;

			llc->hide();

			break;
		}

		i++;
	}

	if(idx < 0) {
		return;
	}

	m->local_libraries.removeAt(idx);

	if(m->local_libraries.isEmpty()) {
		m->empty_library = new EmptyLibraryContainer();
	}

	emit sig_libraries_changed();

	if(m->current_library == removed_llc)
	{
		if(m->local_libraries.isEmpty()) {
			set_current_library(m->empty_library);
		}

		else {
			set_current_library(m->local_libraries.first());
		}
	}


	emit sig_libraries_changed();
}

void PluginHandler::local_library_moved(LibraryId id, int from, int to)
{
	Q_UNUSED(id)

	m->local_libraries.move(from, to);

	emit sig_libraries_changed();
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

ContainerList PluginHandler::get_libraries() const
{
	return m->all_libraries();
}

