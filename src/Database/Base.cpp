/* AbstractDatabase.cpp */

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

#include "Database/Base.h"
#include "Database/Module.h"
#include "Database/Query.h"
#include "Database/Fixes.h"

#include "Utils/FileSystem.h"
#include "Utils/Logger/Logger.h"
#include "Utils/StandardPaths.h"
#include "Utils/Utils.h"

#include <QFile>
#include <QDir>

namespace
{
	constexpr const auto* ClassName = "DB::Base";

	bool checkSayonaraDir()
	{
		auto success = QDir().cd(Util::xdgConfigPath());
		if(!success)
		{
			spLog(Log::Error, ClassName) << "Could not change to .Sayonara dir";
			return false;
		}

		return true;
	}

	bool copyDatabaseFile(const Util::FileSystemPtr& fileSystem, const QString& sourceFile, const QString& targetFile)
	{
		spLog(Log::Info, ClassName) << "Copy " << sourceFile << " to " << targetFile;

		const auto success = fileSystem->copyFile(sourceFile, targetFile);
		if(success)
		{
			auto f = QFile(targetFile);
			f.setPermissions(f.permissions() |
			                 QFile::Permission::WriteOwner | QFile::Permission::WriteUser |
			                 QFile::Permission::ReadOwner | QFile::Permission::ReadUser);
		}

		return success;
	}

	bool createDatabase(const Util::FileSystemPtr& fileSystem, const QString& connectionName,
	                    const QString& sourceDatabaseFile)
	{
		if(!checkSayonaraDir())
		{
			return false;
		}

		if(fileSystem->exists(connectionName))
		{
			return true;
		}

		spLog(Log::Info, ClassName) << "Database " << connectionName << " not existent yet";

		const auto databaseCreated = copyDatabaseFile(fileSystem, sourceDatabaseFile, connectionName);
		if(databaseCreated)
		{
			spLog(Log::Info, ClassName) << "DB file has been copied to " << connectionName;
		}

		else
		{
			spLog(Log::Error, ClassName) << "Fatal Error: could not copy DB file to " << connectionName;
		}

		return databaseCreated;
	}

	bool checkDatabase(const Util::FileSystemPtr& fileSystem, const QString& connectionName,
	                   const QString& sourceDatabaseFile)
	{
		if(!fileSystem->exists(connectionName))
		{
			spLog(Log::Info, ClassName) << "Database not existent. Creating database...";
			return createDatabase(fileSystem, connectionName, sourceDatabaseFile);
		}

		return true;
	}
}

namespace DB
{
	struct Base::Private
	{
		Util::FileSystemPtr fileSystem {Util::FileSystem::create()};
		QString filename;            // player.db
		QString connectionName;    // /home/user/.Sayonara/player.db
		DbId databaseId;
		bool initialized {false};

		Private(const DbId databaseId, const QString& sourceDirectory, const QString& targetDirectory,
		        const QString& filename) :
			filename(filename),
			connectionName {QDir(targetDirectory).absoluteFilePath(filename)},
			databaseId(databaseId)
		{
			if(!fileSystem->exists(targetDirectory))
			{
				fileSystem->createDirectories(targetDirectory);
			}

			const auto sourceDatabaseFile = QDir(sourceDirectory).absoluteFilePath(filename);
			initialized = checkDatabase(fileSystem, connectionName, sourceDatabaseFile);
		}
	};

	Base::Base(const DbId databaseId, const QString& sourceDirectory, const QString& targetDirectory,
	           const QString& filename, Fixes* fixes, QObject* parent) :
		QObject(parent),
		DB::Module(createConnectionName(targetDirectory, filename), databaseId),
		m {Pimpl::make<Private>(databaseId, sourceDirectory, targetDirectory, filename)}
	{
		if(!isInitialized())
		{
			spLog(Log::Error, this) << "Database is not open";
		}

		else if(fixes)
		{
			fixes->applyFixes();
		}
	}

	DB::Base::~Base() = default;

	bool Base::isInitialized() const { return m->initialized && db().isOpen(); }

	bool Base::closeDatabase()
	{
		if(!QSqlDatabase::isDriverAvailable("QSQLITE"))
		{
			return false;
		}

		auto database = db();
		const auto connectionName = database.connectionName();
		if(!QSqlDatabase::connectionNames().contains(connectionName))
		{
			return false;
		}

		spLog(Log::Info, this) << "close database " << m->filename << " (" << connectionName << ")...";

		if(database.isOpen())
		{
			database.close();
		}

		QSqlDatabase::removeDatabase(connectionName);

		return true;
	}

	QString createConnectionName(const QString& targetDirectory, const QString& filename)
	{
		return QDir(targetDirectory).absoluteFilePath(filename);
	}
}