/* LibraryManager.cpp */

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

#include "LibraryManager.h"
#include "Components/Library/LocalLibrary.h"

#include "Database/Connector.h"
#include "Database/Library.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QFile>
#include <QMap>
#include <QObject>
#include <QString>

namespace File=::Util::File;
namespace Algorithm=Util::Algorithm;

using Library::Manager;
using Library::Info;

using OrderMap=QMap<LibraryId, int>;

struct Manager::Private
{

public:
	QMap<LibraryId, LocalLibrary*> libraryMap;
	QList<Info> allLibraries;

	Private() {}

	bool checkNewPath(const QString& path, LibraryId libraryId=-5) const
	{
		if(path.isEmpty()){
			return false;
		}

		const QString sayonaraPath = ::Util::sayonaraPath("Libraries");
		if(path.contains(sayonaraPath, Qt::CaseInsensitive)){
			return false;
		}

		for(const Info& info : allLibraries)
		{
			if(info.id() == libraryId){
				continue;
			}

			if(Util::File::cleanFilename(info.path()) == Util::File::cleanFilename(path)){
				return false;
			}
		}

		return true;
	}


	LibraryId getNextId() const
	{
		LibraryId id=0;
		QList<LibraryId> ids;

		for(const Info& li : allLibraries)
		{
			ids << li.id();
		}

		while(ids.contains(id))
		{
			id++;
		}

		return id;
	}


	Info getLibraryInfo(LibraryId id)
	{
		for(const Info& info : Algorithm::AsConst(allLibraries))
		{
			if(info.id() == id){
				return info;
			}
		}

		return Info();
	}

	Info getLibraryInfoByPath(const QString& path)
	{
		Info ret;

		for(const Info& info : Algorithm::AsConst(allLibraries))
		{
			if( path.startsWith(info.path()) &&
				path.length() > ret.path().length())
			{
				ret = info;
			}
		}

		return ret;
	}

	OrderMap orderMap() const
	{
		OrderMap orderMap;
		int i=0;
		for(const ::Library::Info& info : Algorithm::AsConst(allLibraries))
		{
			orderMap[info.id()] = i;
			i++;
		}

		return orderMap;
	}
};


Manager::Manager() :
	QObject()
{
	m = Pimpl::make<Private>();

	reset();
}

Manager::~Manager() = default;

void Manager::reset()
{
	DB::Library* ldb = DB::Connector::instance()->libraryConnector();
	m->allLibraries = ldb->getAllLibraries();

	if(m->allLibraries.isEmpty())
	{
		m->allLibraries = GetSetting(Set::Lib_AllLibraries);
		int index = 0;
		for(const Library::Info& info : Algorithm::AsConst(m->allLibraries))
		{
			spLog(Log::Info, this) << "All libraries are empty: Insert " << info.toString();
			ldb->insertLibrary(info.id(), info.name(), info.path(), index);
			index ++;
		}

		SetSetting(Set::Lib_AllLibraries, QList<::Library::Info>());
	}

	if(m->allLibraries.isEmpty())
	{
		QString old_path = GetSetting(Set::Lib_Path);

		if(!old_path.isEmpty())
		{
			Info info("Local Library", old_path, 0);
			ldb->insertLibrary(0, info.name(), info.path(), 0);

			m->allLibraries << info;
		}

		SetSetting(Set::Lib_Path, QString());
	}

	for(int i=m->allLibraries.size() - 1; i>=0; i--)
	{
		if(!m->allLibraries[i].valid())
		{
			m->allLibraries.removeAt(i);
		}
	}

	for(const Library::Info& info : m->allLibraries)
	{
		DB::Connector::instance()->registerLibraryDatabase(info.id());
	}
}


LibraryId Manager::addLibrary(const QString& name, const QString& path)
{
	if( (!m->checkNewPath(path)) || (name.isEmpty()) )
	{
		return -1;
	}

	LibraryId id = m->getNextId();
	Info info(name, path, id);

	m->allLibraries << info;

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->registerLibraryDatabase(id);
	lib_db->deleteAllTracks(false); // maybe some corpses from earlier days

	DB::Library* ldb = db->libraryConnector();
	bool success = ldb->insertLibrary(id, name, path, 0);
	if(!success){
		return -1;
	}

	success = ldb->reorderLibraries(m->orderMap());
	if(!success){
		return -1;
	}

	emit sigAdded(id);

	return id;
}

bool Manager::renameLibrary(LibraryId id, const QString& new_name)
{
	auto it = Algorithm::find(m->allLibraries, [&id, &new_name](const Info& info)
	{
		return ((info.id() == id) &&
				(info.name() != new_name));
	});

	if(it == m->allLibraries.end())
	{
		spLog(Log::Warning, this) << "Cannot rename library (1)";
		return false;
	}

	Info old_info = *it;
	Info new_info = Info(new_name, old_info.path(), old_info.id());
	*it = new_info;

	auto* db = DB::Connector::instance();
	DB::Library* ldb = db->libraryConnector();
	bool success = ldb->editLibrary(old_info.id(), new_name, old_info.path());

	if(success)
	{
		emit sigRenamed(id);
	}

	else {
		spLog(Log::Warning, this) << "Cannot rename library (2)";
	}

	return success;
}


bool Manager::removeLibrary(LibraryId id)
{
	LocalLibrary* local_library = m->libraryMap[id];
	if(local_library)
	{
		delete local_library; local_library=nullptr;
	}

	m->libraryMap.remove(id);

	auto* db = DB::Connector::instance();
	db->deleteLibraryDatabase(id);

	DB::Library* ldb = db->libraryConnector();
	bool success = ldb->removeLibrary(id);

	OrderMap order_map = m->orderMap();
	ldb->reorderLibraries(order_map);

	m->allLibraries = ldb->getAllLibraries();

	if(success){
		emit sigRemoved(id);
	}

	return success;
}

bool Manager::moveLibrary(int from, int to)
{
	DB::Library* ldb = DB::Connector::instance()->libraryConnector();

	m->allLibraries.move(from, to);

	OrderMap order_map = m->orderMap();
	ldb->reorderLibraries(order_map);

	m->allLibraries = ldb->getAllLibraries();

	if(Util::between(to, m->allLibraries))
	{
		LibraryId id = m->allLibraries[to].id();
		emit sigMoved(id, from, to);
	}

	return (m->allLibraries.size() > 0);
}

bool Manager::changeLibraryPath(LibraryId id, const QString& new_path)
{
	if(!m->checkNewPath(new_path, id)){
		return false;
	}

	auto it = Algorithm::find(m->allLibraries, [&id](const Info& info){
		return (id == info.id());
	});

	if(it == m->allLibraries.end()){
		return false;
	}

	Info old_info = *it;

	Info new_info(old_info.name(), new_path, old_info.id());
	*it = new_info;

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->libraryDatabase(id, db->databaseId());
	if(lib_db->libraryId() >= 0)
	{
		lib_db->deleteAllTracks(false);
	}

	if(m->libraryMap.contains(id))
	{
		LocalLibrary* ll = m->libraryMap[id];
		if(ll){
			ll->refetch();
		}
	}

	DB::Library* ldb = db->libraryConnector();
	bool success = ldb->editLibrary(old_info.id(), old_info.name(), new_path);

	if(success){
		emit sigPathChanged(id);
	}

	else {
		spLog(Log::Warning, this) << "Cannot change library path";
		success = false;
	}

	return success;
}


QString Manager::requestLibraryName(const QString& path)
{
	QDir d(path);
	return ::Util::stringToFirstUpper(d.dirName());
}

QList<Info> Manager::allLibraries() const
{
	return m->allLibraries;
}

int Manager::count() const
{
	return m->allLibraries.size();
}

Info Manager::libraryInfo(LibraryId id) const
{
	return m->getLibraryInfo(id);
}

Library::Info Manager::libraryInfoByPath(const QString& path) const
{
	return m->getLibraryInfoByPath(path);
}

LocalLibrary* Manager::libraryInstance(LibraryId id)
{
	LocalLibrary* lib = nullptr;
	auto it = Algorithm::find(m->allLibraries, [&id](const Info& info){
		return (info.id() == id);
	});

	if( it != m->allLibraries.end() &&
		m->libraryMap.contains(id) )
	{
		lib = m->libraryMap[id];
	}

	if(lib == nullptr)
	{
		lib = new LocalLibrary(id);
		m->libraryMap[id] = lib;
	}

	return lib;
}
