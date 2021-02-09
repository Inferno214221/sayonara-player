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
#include <QMap>
#include <QObject>
#include <QString>

namespace File = ::Util::File;
namespace Algorithm = Util::Algorithm;

using Library::Manager;
using Library::Info;
using OrderMap = QMap<LibraryId, int>;

constexpr const auto InvalidLibraryId = -5;

struct Manager::Private
{
	public:
		QMap<LibraryId, LocalLibrary*> libraryMap;
		Playlist::Handler* playlistHandler;
		QList<Info> libraries;
		DB::Connector* database;
		DB::Library* libraryConnector;

		Private(Playlist::Handler* playlistHandler) :
			playlistHandler {playlistHandler},
			database {DB::Connector::instance()},
			libraryConnector {database->libraryConnector()} {}

		bool checkNewPath(const QString& path, LibraryId libraryId = InvalidLibraryId) const
		{
			if(path.isEmpty())
			{
				return false;
			}

			for(const auto& info : libraries)
			{
				if(info.id() != libraryId)
				{
					if(Util::File::isSamePath(info.path(), path))
					{
						return false;
					}

					else if(Util::File::isSubdir(path, info.path()))
					{
						return false;
					}
				}
			}

			return true;
		}

		LibraryId getNextId() const
		{
			const auto it =
				std::max_element(libraries.begin(), libraries.end(), [](const auto& info1, const auto& info2) {
					return (info1.id() < info2.id());
				});

			return (it != libraries.end()) ? it->id() + 1 : 0;
		}

		OrderMap orderMap() const
		{
			OrderMap orderMap;
			int i = 0;
			for(auto it = libraries.begin(); it != libraries.end(); it++, i++)
			{
				orderMap[it->id()] = i;
			}

			return orderMap;
		}

		bool refetchLibraries()
		{
			const auto orderMap = this->orderMap();
			const auto success = this->libraryConnector->reorderLibraries(orderMap);
			this->libraries = this->libraryConnector->getAllLibraries();

			return success;
		}
};

Manager::Manager(Playlist::Handler* playlistHandler) :
	QObject()
{
	m = Pimpl::make<Private>(playlistHandler);
	reset();
}

Manager::~Manager() = default;

void Manager::reset()
{
	m->libraries = m->libraryConnector->getAllLibraries();

	if(m->libraries.isEmpty())
	{
		m->libraries = GetSetting(Set::Lib_AllLibraries);

		int i = 0;
		for(auto it = m->libraries.begin(); it != m->libraries.end(); it++, i++)
		{
			m->libraryConnector->insertLibrary(it->id(), it->name(), it->path(), i);
			spLog(Log::Info, this) << "All libraries are empty: Insert " << it->toString();
		}

		SetSetting(Set::Lib_AllLibraries, QList<::Library::Info>());
	}

	if(m->libraries.isEmpty())
	{
		const auto oldPath = GetSetting(Set::Lib_Path);
		if(!oldPath.isEmpty())
		{
			const auto info = Info("Local Library", oldPath, 0);
			m->libraryConnector->insertLibrary(0, info.name(), info.path(), 0);
			m->libraries << info;
		}

		SetSetting(Set::Lib_Path, QString());
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
	if((!m->checkNewPath(path)) || (name.isEmpty()))
	{
		return -1;
	}

	const auto id = m->getNextId();
	m->libraries << Info(name, path, id);

	auto* libraryDatabase = m->database->registerLibraryDatabase(id);
	libraryDatabase->deleteAllTracks(false); // maybe some corpses from earlier days

	const auto inserted = m->libraryConnector->insertLibrary(id, name, path, 0);
	if(inserted)
	{
		const auto reordered = m->libraryConnector->reorderLibraries(m->orderMap());
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
	if(newName.isEmpty())
	{
		return false;
	}

	const auto nameExists = Util::Algorithm::contains(m->libraries, [&](const auto& info) {
		return (info.name() == newName);
	});

	if(nameExists)
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

	*it = Info(newName, it->path(), it->id());

	const auto edited = m->libraryConnector->editLibrary(it->id(), newName, it->path());
	if(edited)
	{
		emit sigRenamed(id);
		return true;
	}

	spLog(Log::Warning, this) << "Cannot rename library: Database error";
	return false;
}

bool Manager::removeLibrary(LibraryId id)
{
	const auto exists = (Util::Algorithm::contains(m->libraries, [&](const auto& info) {
		return (info.id() == id);
	}));

	if(m->libraryMap[id])
	{
		delete m->libraryMap[id];
	}

	m->libraryMap.remove(id);
	m->database->deleteLibraryDatabase(id);

	const auto removed = m->libraryConnector->removeLibrary(id);
	if(removed)
	{
		m->refetchLibraries();
		emit sigRemoved(id);

		return exists;
	}

	return false;
}

bool Manager::moveLibrary(int from, int to)
{
	if(from < 0 || from >= count() || to < 0 || to >= count())
	{
		return false;
	}

	const auto id = m->libraries[from].id();

	m->libraries.move(from, to);
	m->refetchLibraries();

	emit sigMoved(id, from, to);

	return (!m->libraries.isEmpty());
}

bool Manager::changeLibraryPath(LibraryId id, const QString& newPath)
{
	if(!m->checkNewPath(newPath, id))
	{
		return false;
	}

	const auto it = Algorithm::find(m->libraries, [&id](const Info& info) {
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
		auto* localLibrary = m->libraryMap[id];
		if(localLibrary)
		{
			localLibrary->refetch();
		}
	}

	const auto success = m->libraryConnector->editLibrary(oldInfo.id(), oldInfo.name(), newPath);
	if(success)
	{
		emit sigPathChanged(id);
	}

	else
	{
		spLog(Log::Warning, this) << "Cannot change library path";
	}

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
	const auto it = Algorithm::find(m->libraries, [&](const auto& info) {
		return (info.id() == id);
	});

	return (it != m->libraries.end()) ? *it : Info();
}

Library::Info Manager::libraryInfoByPath(const QString& path) const
{
	Info ret;

	for(auto it = m->libraries.begin(); it != m->libraries.end(); it++)
	{
		if(path.startsWith(it->path()) && path.length() > ret.path().length())
		{
			ret = *it;
		}
	}

	return ret;
}

LocalLibrary* Manager::libraryInstance(LibraryId id)
{
	LocalLibrary* localLibrary = nullptr;
	auto it = Algorithm::find(m->libraries, [&id](const Info& info) {
		return (info.id() == id);
	});

	if(it != m->libraries.end() && m->libraryMap.contains(id))
	{
		localLibrary = m->libraryMap[id];
	}

	if(localLibrary == nullptr)
	{
		localLibrary = new LocalLibrary(this, id, m->playlistHandler);
		m->libraryMap[id] = localLibrary;
	}

	return localLibrary;
}
