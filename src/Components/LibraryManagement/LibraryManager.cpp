/* LibraryManager.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

namespace
{
	using Library::Manager;
	using Library::Info;
	using OrderMap = QMap<LibraryId, int>;

	constexpr const auto InvalidLibraryId = -5;

	Library::Info getLibraryInfo(const QList<Info>& libraries, std::function<bool(const Library::Info&)>&& function)
	{
		const auto it = Util::Algorithm::find(libraries, function);
		return (it != libraries.end()) ? *it : Info {};
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

		for(const auto& info: libraries) // NOLINT(readability-use-anyofallof)
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
		       ? it->id() + 1 // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
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

	class LibraryManagerImpl :
		public Library::Manager
	{
		public:
			explicit LibraryManagerImpl(LibraryPlaylistInteractor* playlistInteractor) :
				m_playlistInteractor {playlistInteractor},
				m_database {DB::Connector::instance()},
				m_libraryConnector {m_database->libraryConnector()},
				m_genericLibrary {new LocalLibrary(this, -1, m_playlistInteractor)}
			{
				reset();
			}

			~LibraryManagerImpl() override = default;

			LibraryId addLibrary(const QString& name, const QString& path) override
			{
				if(!checkNewPath(path, m_libraries) || !checkNewName(name, m_libraries))
				{
					return -1;
				}

				const auto id = getNextId(m_libraries);
				m_libraries << Info(name, path, id);

				auto* libraryDatabase = m_database->registerLibraryDatabase(id);
				libraryDatabase->deleteAllTracks(false); // maybe some corpses from earlier days

				const auto inserted = m_libraryConnector->insertLibrary(id, name, path, 0);
				if(inserted)
				{
					const auto orderMap = createOrderMap(m_libraries);
					const auto reordered = m_libraryConnector->reorderLibraries(orderMap);
					if(reordered)
					{
						emit sigAdded(id);
						return id;
					}
				}

				return -1;
			}

			bool renameLibrary(LibraryId id, const QString& newName) override
			{
				if(!checkNewName(newName, m_libraries))
				{
					spLog(Log::Warning, this) << "Cannot rename library: Name already exists";
					return false;
				}

				auto it = Util::Algorithm::find(m_libraries, [&](const auto& info) {
					return (info.id() == id);
				});

				if(it == m_libraries.end())
				{
					spLog(Log::Warning, this) << "Cannot rename library: Cannot find id";
					return false;
				}

				auto newInfo = Info(newName, it->path(), it->id());
				const auto edited = m_libraryConnector->editLibrary(newInfo.id(), newInfo.name(), newInfo.path());
				if(!edited)
				{
					spLog(Log::Warning, this) << "Cannot rename library: Database error";
					return false;
				}

				*it = newInfo;
				emit sigRenamed(id);

				return true;
			}

			bool removeLibrary(LibraryId id) override
			{
				const auto exists = (Util::Algorithm::contains(m_libraries, [&](const auto& info) {
					return (info.id() == id);
				}));

				m_libraryConnector->removeLibrary(id);
				m_database->deleteLibraryDatabase(id);
				m_libraries = refetchLibraries(m_libraries);

				emit sigRemoved(id);

				auto* localLibrary = m_libraryMap.take(id);
				delete localLibrary;

				return exists;
			}

			bool moveLibrary(int from, int to) override
			{
				if(!Util::between(from, count()) || !Util::between(to, count()))
				{
					return false;
				}

				const auto id = m_libraries[from].id();

				m_libraries.move(from, to);
				m_libraries = refetchLibraries(m_libraries);

				emit sigMoved(id, from, to);

				return (!m_libraries.isEmpty());
			}

			bool changeLibraryPath(LibraryId id, const QString& newPath) override
			{
				if(!checkNewPath(newPath, m_libraries, id))
				{
					return false;
				}

				const auto it = Util::Algorithm::find(m_libraries, [&](const auto& info) {
					return (id == info.id());
				});

				if(it == m_libraries.end())
				{
					return false;
				}

				const auto oldInfo = *it;
				*it = Info(oldInfo.name(), newPath, oldInfo.id());

				auto* libraryDatabase = m_database->libraryDatabase(id, m_database->databaseId());
				if(libraryDatabase->libraryId() >= 0)
				{
					libraryDatabase->deleteAllTracks(false);
				}

				if(m_libraryMap.contains(id))
				{
					auto* localLibrary = m_libraryMap.value(id);
					localLibrary->refetch();
				}

				const auto success = m_libraryConnector->editLibrary(oldInfo.id(), oldInfo.name(), newPath);
				if(success)
				{
					emit sigPathChanged(id);
				}

				spLog(Log::Warning, this) << "Library path changed: " << success;

				return success;
			}

			[[nodiscard]] QList<Info> allLibraries() const override { return m_libraries; }

			[[nodiscard]] int count() const override { return m_libraries.size(); }

			[[nodiscard]] Info libraryInfo(LibraryId id) const override
			{
				return getLibraryInfo(m_libraries, [&](const auto& info) {
					return (info.id() == id);
				});
			}

			[[nodiscard]] Info libraryInfoByPath(const QString& path) const override
			{
				return getLibraryInfo(m_libraries, [&](const auto& info) {
					return (isSubPath(info.path(), path));
				});
			}

			LocalLibrary* libraryInstance(LibraryId id) override
			{
				if(id == -1)
				{
					return m_genericLibrary;
				}

				const auto exists = Util::Algorithm::contains(m_libraries, [&](const auto& info) {
					return (info.id() == id);
				});

				auto* localLibrary = (exists && m_libraryMap.contains(id))
				                     ? m_libraryMap[id]
				                     : new LocalLibrary(this, id, m_playlistInteractor);

				if(!m_libraryMap.contains(id))
				{
					m_libraryMap[id] = localLibrary;
				}

				return localLibrary;
			}

		private:
			void reset()
			{
				m_libraries = m_libraryConnector->getAllLibraries();
				if(m_libraries.isEmpty())
				{
					m_libraries = getLegacyLibraries(m_libraryConnector);
				}

				for(int i = m_libraries.size() - 1; i >= 0; i--)
				{
					if(m_libraries[i].valid())
					{
						m_database->registerLibraryDatabase(m_libraries[i].id());
					}
					else
					{
						m_libraries.removeAt(i);
					}
				}
			}

			QMap<LibraryId, LocalLibrary*> m_libraryMap;
			QList<Info> m_libraries;
			LibraryPlaylistInteractor* m_playlistInteractor;
			DB::Connector* m_database;
			DB::Library* m_libraryConnector;
			LocalLibrary* m_genericLibrary;
	};
}

namespace Library
{
	Manager* Manager::create(LibraryPlaylistInteractor* playlistInteractor)
	{
		return new LibraryManagerImpl(playlistInteractor);
	}

	QString Manager::requestLibraryName(const QString& path)
	{
		return Util::stringToFirstUpper(QDir(path).dirName());
	}
}

