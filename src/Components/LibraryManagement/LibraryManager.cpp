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
#include "Components/Playlist/LibraryPlaylistInteractor.h"
#include "Database/Connector.h"
#include "Database/Library.h"
#include "Database/LibraryDatabase.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

#include <QDir>
#include <QMap>
#include <QObject>
#include <QString>

namespace File = ::Util::File;
namespace Algorithm = Util::Algorithm;

using Library::Manager;
using Library::Info;
using OrderMap = QMap<LibraryId, int>;

namespace
{
	constexpr const auto InvalidLibraryId = -5;

	Library::Info getLibraryInfo(const QList<Info>& libraries, std::function<bool(const Library::Info&)> function)
	{
		const auto it = Algorithm::find(libraries, function);
		return (it != libraries.end())
		       ? *it :
		       Info();
	}

	bool isSubPath(const QString& subPath, const QString& fullPath)
	{
		return Util::File::isSubdir(subPath, fullPath) &&
		       !Util::File::isSamePath(subPath, fullPath);
	}

	bool checkNewPath(const QString& path, const QList<Info>& libraries, LibraryId libraryId = InvalidLibraryId)
	{
		if(path.isEmpty())
		{
			return false;
		}

		for(const auto& info: libraries)
		{
			if(info.id() != libraryId)
			{
				if(Util::File::isSamePath(info.path(), path) ||
				   Util::File::isSubdir(path, info.path()))
				{
					return false;
				}
			}
		}

		return true;
	}

	bool checkNewName(const QString& name, const QList<Info>& libraries)
	{
		if(name.isEmpty())
		{
			return false;
		}

		return !Util::Algorithm::contains(libraries, [&](const auto& info) {
			return (info.name() == name);
		});
	}

	LibraryId getNextId(const QList<Info>& libraries)
	{
		const auto it =
			std::max_element(libraries.begin(), libraries.end(), [](const auto& info1, const auto& info2) {
				return (info1.id() < info2.id());
			});

		return (it != libraries.end())
		       ? it->id() + 1
		       : 0;
	}

	OrderMap createOrderMap(const QList<Info>& libraries)
	{
		OrderMap orderMap;
		int i = 0;
		for(auto it = libraries.begin(); it != libraries.end(); it++, i++)
		{
			orderMap[it->id()] = i;
		}

		return orderMap;
	}

	QList<Info> refetchLibraries(const QList<Info>& libraries)
	{
		auto* libraryConnector = DB::Connector::instance()->libraryConnector();

		const auto orderMap = createOrderMap(libraries);
		libraryConnector->reorderLibraries(orderMap);

		return libraryConnector->getAllLibraries();
	}

	QList<Info> getLegacyLibraries(DB::Library* libraryConnector)
	{
		auto libraries = GetSetting(Set::Lib_AllLibraries);
		if(libraries.isEmpty())
		{
			const auto oldPath = GetSetting(Set::Lib_Path);
			if(!oldPath.isEmpty())
			{
				const auto info = Info("Local Library", oldPath, 0);
				libraries << info;
			}
		}

		int i = 0;
		for(auto it = libraries.begin(); it != libraries.end(); it++, i++)
		{
			libraryConnector->insertLibrary(it->id(), it->name(), it->path(), i);
		}

		SetSetting(Set::Lib_AllLibraries, QList<::Library::Info>());
		SetSetting(Set::Lib_Path, QString());

		return libraries;
	}
}

struct Manager::Private
{
	public:
		QMap<LibraryId, LocalLibrary*> libraryMap;
		QList<Info> libraries;
		LibraryPlaylistInteractor* playlistInteractor;
		DB::Connector* database;
		DB::Library* libraryConnector;

		Private(LibraryPlaylistInteractor* playlistInteractor) :
			playlistInteractor {playlistInteractor},
			database {DB::Connector::instance()},
			libraryConnector {database->libraryConnector()} {}
};

Manager::Manager(LibraryPlaylistInteractor* playlistInteractor) :
	QObject()
{
	m = Pimpl::make<Private>(playlistInteractor);
	reset();
}

Manager::~Manager() = default;

void Manager::reset()
{
	m->libraries = m->libraryConnector->getAllLibraries();
	if(m->libraries.isEmpty())
	{
		m->libraries = getLegacyLibraries(m->libraryConnector);
	}

	for(int i = m->libraries.size() - 1; i >= 0; i--)
	{
		if(m->libraries[i].valid())
		{
			m->database->registerLibraryDatabase(m->libraries[i].id());
		}
		else
		{
			m->libraries.removeAt(i);
		}
	}
}

LibraryId Manager::addLibrary(const QString& name, const QString& path)
{
	if(!checkNewPath(path, m->libraries) || !checkNewName(name, m->libraries))
	{
		return -1;
	}

	const auto id = getNextId(m->libraries);
	m->libraries << Info(name, path, id);

	auto* libraryDatabase = m->database->registerLibraryDatabase(id);
	libraryDatabase->deleteAllTracks(false); // maybe some corpses from earlier days

	const auto inserted = m->libraryConnector->insertLibrary(id, name, path, 0);
	if(inserted)
	{
		const auto orderMap = createOrderMap(m->libraries);
		const auto reordered = m->libraryConnector->reorderLibraries(orderMap);
		if(reordered)
		{
			emit sigAdded(id);
			return id;
		}
	}

	return -1;
}

bool Manager::renameLibrary(LibraryId id, const QString& newName)
{
	if(!checkNewName(newName, m->libraries))
	{
		spLog(Log::Warning, this) << "Cannot rename library: Name already exists";
		return false;
	}

	auto it = Util::Algorithm::find(m->libraries, [&](const auto& info) {
		return (info.id() == id);
	});

	if(it == m->libraries.end())
	{
		spLog(Log::Warning, this) << "Cannot rename library: Cannot find id";
		return false;
	}

	auto newInfo = Info(newName, it->path(), it->id());
	const auto edited = m->libraryConnector->editLibrary(newInfo.id(), newInfo.name(), newInfo.path());
	if(!edited)
	{
		spLog(Log::Warning, this) << "Cannot rename library: Database error";
		return false;
	}

	*it = std::move(newInfo);
	emit sigRenamed(id);

	return true;
}

bool Manager::removeLibrary(LibraryId id)
{
	const auto exists = (Util::Algorithm::contains(m->libraries, [&](const auto& info) {
		return (info.id() == id);
	}));

	m->libraryConnector->removeLibrary(id);
	m->database->deleteLibraryDatabase(id);
	m->libraries = refetchLibraries(m->libraries);

	emit sigRemoved(id);

	auto* localLibrary = m->libraryMap.take(id);
	if(localLibrary)
	{
		delete localLibrary;
	}

	return exists;
}

bool Manager::moveLibrary(int from, int to)
{
	if(!Util::between(from, count()) || !Util::between(to, count()))
	{
		return false;
	}

	const auto id = m->libraries[from].id();

	m->libraries.move(from, to);
	m->libraries = refetchLibraries(m->libraries);

	emit sigMoved(id, from, to);

	return (!m->libraries.isEmpty());
}

bool Manager::changeLibraryPath(LibraryId id, const QString& newPath)
{
	if(!checkNewPath(newPath, m->libraries, id))
	{
		return false;
	}

	const auto it = Algorithm::find(m->libraries, [&](const auto& info) {
		return (id == info.id());
	});

	if(it == m->libraries.end())
	{
		return false;
	}

	const auto oldInfo = std::move(*it);
	*it = Info(oldInfo.name(), newPath, oldInfo.id());

	auto* libraryDatabase = m->database->libraryDatabase(id, m->database->databaseId());
	if(libraryDatabase->libraryId() >= 0)
	{
		libraryDatabase->deleteAllTracks(false);
	}

	if(m->libraryMap.contains(id))
	{
		auto* localLibrary = m->libraryMap.value(id);
		localLibrary->refetch();
	}

	const auto success = m->libraryConnector->editLibrary(oldInfo.id(), oldInfo.name(), newPath);
	if(success)
	{
		emit sigPathChanged(id);
	}

	spLog(Log::Warning, this) << "Library path changed: " << success;

	return success;
}

QString Manager::requestLibraryName(const QString& path)
{
	return Util::stringToFirstUpper(QDir(path).dirName());
}

QList<Info> Manager::allLibraries() const
{
	return m->libraries;
}

int Manager::count() const
{
	return m->libraries.size();
}

Info Manager::libraryInfo(LibraryId id) const
{
	return getLibraryInfo(m->libraries, [&](const auto& info) {
		return (info.id() == id);
	});
}

Library::Info Manager::libraryInfoByPath(const QString& path) const
{
	return getLibraryInfo(m->libraries, [&](const auto& info) {
		return (isSubPath(info.path(), path));
	});
}

LocalLibrary* Manager::libraryInstance(LibraryId id)
{
	if(id == -1)
	{
		static auto* genericLibrary = new LocalLibrary(this, -1, m->playlistInteractor);
		return genericLibrary;
	}

	const auto exists = Util::Algorithm::contains(m->libraries, [&](const auto& info) {
		return (info.id() == id);
	});

	auto* localLibrary = (exists && m->libraryMap.contains(id))
	                     ? m->libraryMap[id]
	                     : new LocalLibrary(this, id, m->playlistInteractor);

	if(!m->libraryMap.contains(id))
	{
		m->libraryMap[id] = localLibrary;
	}

	return localLibrary;
}
