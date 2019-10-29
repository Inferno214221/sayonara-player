/* LibraryManager.cpp */

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
	QMap<LibraryId, LocalLibrary*> lib_map;
	QList<Info> all_libs;

	Private() {}

	bool check_new_path(const QString& path, LibraryId library_id=-5) const
	{
		if(path.isEmpty()){
			return false;
		}

		QString sayonara_path = ::Util::sayonara_path("Libraries");
		if(path.contains(sayonara_path, Qt::CaseInsensitive)){
			return false;
		}

		for(const Info& info : all_libs)
		{
			if(info.id() == library_id){
				continue;
			}

			if(Util::File::clean_filename(info.path()) == Util::File::clean_filename(path)){
				return false;
			}
		}

		return true;
	}


	LibraryId get_next_id() const
	{
		LibraryId id=0;
		QList<LibraryId> ids;

		for(const Info& li : all_libs)
		{
			ids << li.id();
		}

		while(ids.contains(id))
		{
			id++;
		}

		return id;
	}


	Info get_library_info(LibraryId id)
	{
		for(const Info& info : Algorithm::AsConst(all_libs))
		{
			if(info.id() == id){
				return info;
			}
		}

		return Info();
	}

	Info get_library_info_by_path(const QString& path)
	{
		Info ret;

		for(const Info& info : Algorithm::AsConst(all_libs))
		{
			if( path.startsWith(info.path()) &&
				path.length() > ret.path().length())
			{
				ret = info;
			}
		}

		return ret;
	}


	Info get_library_info_by_sympath(const QString& sympath)
	{
		Info ret;

		for(const Info& info : Algorithm::AsConst(all_libs))
		{
			if( sympath.startsWith(info.symlink_path()) &&
				sympath.length() > ret.symlink_path().length())
			{
				ret = info;
			}
		}

		return ret;
	}

	void init_symlinks()
	{
		QString dir = ::Util::sayonara_path("Libraries");
		QDir d(dir);

		QFileInfoList symlinks = d.entryInfoList(QDir::NoFilter | QDir::NoDotAndDotDot);
		for(const QFileInfo& symlink : symlinks)
		{
			if(symlink.isSymLink()) {
				QFile::remove(symlink.absoluteFilePath());
			}
		}

		::Util::File::create_directories(dir);

		for(const Info& info : Algorithm::AsConst(all_libs))
		{
			QString target = info.symlink_path();

			if(!(File::exists(target))){
				File::create_symlink(info.path(), target);
			}
		}
	}

	OrderMap order_map() const
	{
		OrderMap order_map;
		int i=0;
		for(const ::Library::Info& info : Algorithm::AsConst(all_libs))
		{
			order_map[info.id()] = i;
			i++;
		}

		return order_map;
	}
};


Manager::Manager() :
	QObject(),
	DB::ConnectorConsumer()
{
	m = Pimpl::make<Private>();

	reset();
	m->init_symlinks();
}

Manager::~Manager() = default;

void Manager::reset()
{
	DB::Library* ldb = db_connector()->library_connector();
	m->all_libs = ldb->get_all_libraries();

	if(m->all_libs.isEmpty())
	{
		m->all_libs = GetSetting(Set::Lib_AllLibraries);
		int index = 0;
		for(const Library::Info& info : Algorithm::AsConst(m->all_libs))
		{
			sp_log(Log::Info, this) << "All libraries are empty: Insert " << info.toString();
			ldb->insert_library(info.id(), info.name(), info.path(), index);
			index ++;
		}

		SetSetting(Set::Lib_AllLibraries, QList<::Library::Info>());
	}

	if(m->all_libs.isEmpty())
	{
		QString old_path = GetSetting(Set::Lib_Path);

		if(!old_path.isEmpty())
		{
			Info info("Local Library", old_path, 0);
			ldb->insert_library(0, info.name(), info.path(), 0);

			m->all_libs << info;
		}

		SetSetting(Set::Lib_Path, QString());
	}

	for(int i=m->all_libs.size() - 1; i>=0; i--)
	{
		if(!m->all_libs[i].valid())
		{
			m->all_libs.removeAt(i);
		}

		else
		{
			if(!File::exists(m->all_libs[i].symlink_path()))
			{
				File::create_symlink(m->all_libs[i].path(), m->all_libs[i].symlink_path());
			}
		}
	}

	for(const Library::Info& info : m->all_libs)
	{
		db_connector()->register_library_db(info.id());
	}
}


LibraryId Manager::add_library(const QString& name, const QString& path)
{
	if( (!m->check_new_path(path)) || (name.isEmpty()) )
	{
		return -1;
	}

	LibraryId id = m->get_next_id();
	Info info(name, path, id);

	m->all_libs << info;

	DB::Connector* db = db_connector();
	DB::LibraryDatabase* lib_db = db->register_library_db(id);
	lib_db->deleteAllTracks(false); // maybe some corpses from earlier days

	DB::Library* ldb = db->library_connector();
	bool success = ldb->insert_library(id, name, path, 0);
	if(!success){
		return -1;
	}

	::Util::File::create_symlink(info.path(), info.symlink_path());

	success = ldb->reorder_libraries(m->order_map());
	if(!success){
		return -1;
	}

	emit sig_added(id);

	return id;
}

bool Manager::rename_library(LibraryId id, const QString& new_name)
{
	auto it = Algorithm::find(m->all_libs, [&id, &new_name](const Info& info)
	{
		return ((info.id() == id) &&
				(info.name() != new_name));
	});

	if(it == m->all_libs.end())
	{
		sp_log(Log::Warning, this) << "Cannot rename library (1)";
		return false;
	}

	Info old_info = *it;
	Info new_info = Info(new_name, old_info.path(), old_info.id());
	*it = new_info;

	DB::Connector* db = db_connector();
	DB::Library* ldb = db->library_connector();
	bool success = ldb->edit_library(old_info.id(), new_name, old_info.path());

	if(success)
	{
		QFile::remove(old_info.symlink_path());
		File::create_symlink(old_info.path(), new_info.symlink_path());
		emit sig_renamed(id);
	}

	else {
		sp_log(Log::Warning, this) << "Cannot rename library (2)";
	}

	return success;
}


bool Manager::remove_library(LibraryId id)
{
	LocalLibrary* local_library = m->lib_map[id];
	if(local_library)
	{
		delete local_library; local_library=nullptr;
	}

	m->lib_map.remove(id);

	Library::Info info = m->get_library_info(id);
	if(info.valid()){
		QFile::remove(info.symlink_path());
	}

	DB::Connector* db = db_connector();
	db->delete_library_db(id);

	DB::Library* ldb = db->library_connector();
	bool success = ldb->remove_library(id);

	OrderMap order_map = m->order_map();
	ldb->reorder_libraries(order_map);

	m->all_libs = ldb->get_all_libraries();

	if(success){
		emit sig_removed(id);
	}

	return success;
}

bool Manager::move_library(int from, int to)
{
	DB::Library* ldb = db_connector()->library_connector();

	m->all_libs.move(from, to);

	OrderMap order_map = m->order_map();
	ldb->reorder_libraries(order_map);

	m->all_libs = ldb->get_all_libraries();

	if(Util::between(to, m->all_libs))
	{
		LibraryId id = m->all_libs[to].id();
		emit sig_moved(id, from, to);
	}

	return (m->all_libs.size() > 0);
}

bool Manager::change_library_path(LibraryId id, const QString& new_path)
{
	if(!m->check_new_path(new_path, id)){
		return false;
	}

	auto it = Algorithm::find(m->all_libs, [&id](const Info& info){
		return (id == info.id());
	});

	if(it == m->all_libs.end()){
		return false;
	}

	Info old_info = *it;

	Info new_info(old_info.name(), new_path, old_info.id());
	*it = new_info;

	QFile::remove(old_info.symlink_path());
	File::create_symlink(new_info.path(), new_info.symlink_path());

	DB::Connector* db = db_connector();
	DB::LibraryDatabase* lib_db = db->library_db(id, db->db_id());
	if(lib_db->library_id() >= 0)
	{
		lib_db->deleteAllTracks(false);
	}

	if(m->lib_map.contains(id))
	{
		LocalLibrary* ll = m->lib_map[id];
		if(ll){
			ll->refetch();
		}
	}

	DB::Library* ldb = db->library_connector();
	bool success = ldb->edit_library(old_info.id(), old_info.name(), new_path);

	if(success){
		emit sig_path_changed(id);
	}

	else {
		sp_log(Log::Warning, this) << "Cannot change library path";
		success = false;
	}

	return success;
}


QString Manager::request_library_name(const QString& path)
{
	QDir d(path);
	return ::Util::cvt_str_to_first_upper(d.dirName());
}

QList<Info> Manager::all_libraries() const
{
	return m->all_libs;
}

int Manager::count() const
{
	return m->all_libs.size();
}

Info Manager::library_info(LibraryId id) const
{
	return m->get_library_info(id);
}

Library::Info Manager::library_info_by_path(const QString& path) const
{
	return m->get_library_info_by_path(path);
}

Library::Info Manager::library_info_by_sympath(const QString& sympath) const
{
	return m->get_library_info_by_sympath(sympath);
}

LocalLibrary* Manager::library_instance(LibraryId id)
{
	LocalLibrary* lib = nullptr;
	auto it = Algorithm::find(m->all_libs, [&id](const Info& info){
		return (info.id() == id);
	});

	if( it != m->all_libs.end() &&
		m->lib_map.contains(id) )
	{
		lib = m->lib_map[id];
	}

	if(lib == nullptr)
	{
		lib = new LocalLibrary(id);
		m->lib_map[id] = lib;
	}

	return lib;
}
