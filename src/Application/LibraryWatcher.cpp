#include "LibraryWatcher.h"
#include "Interfaces/LibraryInterface/LibraryPluginHandler.h"
#include "Components/Library/LibraryManager.h"
#include "Gui/Library/LocalLibraryContainer.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Algorithm.h"

#include <QMap>

using namespace Library;

struct LocalLibraryPluginHandler::Private
{
	QList<Info> infos;

	Private()
	{
		infos = Manager::instance()->all_libraries();
	}
};

LocalLibraryPluginHandler::LocalLibraryPluginHandler(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	Manager* manager = Manager::instance();

	connect(manager, &Manager::sig_added, this, &LocalLibraryPluginHandler::library_added);
	connect(manager, &Manager::sig_moved, this, &LocalLibraryPluginHandler::library_moved);
	connect(manager, &Manager::sig_renamed, this, &LocalLibraryPluginHandler::library_renamed);
	connect(manager, &Manager::sig_removed, this, &LocalLibraryPluginHandler::library_removed);
}

LocalLibraryPluginHandler::~LocalLibraryPluginHandler() = default;

QList<Container*> LocalLibraryPluginHandler::get_local_library_containers() const
{
	QList<Container*> containers;

	for(const Info& info : m->infos)
	{
		containers << new LocalLibraryContainer(info);
	}

	return containers;
}

void LocalLibraryPluginHandler::library_added(LibraryId id)
{
	PluginHandler* lph = PluginHandler::instance();
	Info info = Manager::instance()->library_info(id);

	Container* c = new LocalLibraryContainer(info);
	lph->add_local_library(c);

	m->infos = Manager::instance()->all_libraries();
}

void LocalLibraryPluginHandler::library_moved(LibraryId id, int from, int to)
{
	Q_UNUSED(id)

	PluginHandler* lph = PluginHandler::instance();
	lph->move_local_library(from, to);

	m->infos = Manager::instance()->all_libraries();
}

void LocalLibraryPluginHandler::library_renamed(LibraryId id)
{
	PluginHandler* lph = PluginHandler::instance();

	Info info = Manager::instance()->library_info(id);
	int idx = Util::Algorithm::indexOf(m->infos, [id](const Info& info){
		return (info.id() == id);
	});

	if(Util::between(idx, m->infos))
	{
		QString old_name = m->infos[idx].name();
		QString new_name = info.name();

		lph->rename_local_library(old_name, new_name);
	}

	m->infos = Manager::instance()->all_libraries();
}

void LocalLibraryPluginHandler::library_removed(LibraryId id)
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
