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

#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/StandardPaths.h"
#include "Utils/Utils.h"

#include <QFile>
#include <QDir>

using DB::Base;
using DB::Query;

namespace FileUtils = ::Util::File;

struct Base::Private
{
	QString sourceDirectory;
	QString targetDirectory;
	QString filename;            // player.db
	QString connectionName;    // /home/user/.Sayonara/player.db
	DbId databaseId;

	bool initialized;

	Private(DbId databaseId, const QString& sourceDirectory, const QString& targetDirectory, const QString& filename) :
		sourceDirectory(sourceDirectory),
		targetDirectory(targetDirectory),
		filename(filename),
		databaseId(databaseId)
	{
		connectionName = targetDirectory + "/" + filename;

		if(!Util::File::exists(targetDirectory))
		{
			Util::File::createDirectories(targetDirectory);
		}
	}
};

Base::Base(DbId databaseId, const QString& sourceDirectory, const QString& targetDirectory, const QString& filename,
           QObject* parent) :
	QObject(parent),
	DB::Module(targetDirectory + "/" + filename, databaseId)
{
	m = Pimpl::make<Private>(databaseId, sourceDirectory, targetDirectory, filename);
	bool success = FileUtils::exists(m->connectionName);

	if(!success)
	{
		spLog(Log::Info, this) << "Database not existent. Creating database...";
		success = createDatabase();
	}

	m->initialized = success && this->db().isOpen();

	if(!m->initialized)
	{
		spLog(Log::Error, this) << "Database is not open";
	}
}

DB::Base::~Base() = default;

bool Base::isInitialized()
{
	return m->initialized;
}

bool Base::closeDatabase()
{
	if(!QSqlDatabase::isDriverAvailable("QSQLITE"))
	{
		return false;
	}

	QString connection_name;
	{
		QSqlDatabase database = db();
		connection_name = database.connectionName();
		QStringList connection_names = QSqlDatabase::connectionNames();
		if(!connection_names.contains(connection_name))
		{
			return false;
		}

		spLog(Log::Info, this) << "close database " << m->filename << " (" << connection_name << ")...";

		if(database.isOpen())
		{
			database.close();
		}
	}

	QSqlDatabase::removeDatabase(connection_name);

	return true;
}

bool Base::createDatabase()
{
	//if ret is still not true we are not able to create the directory
	bool success = QDir().cd(Util::xdgConfigPath());
	if(!success)
	{
		spLog(Log::Error, this) << "Could not change to .Sayonara dir";
		return false;
	}

	const auto sourceDatabasefile = QDir(m->sourceDirectory).absoluteFilePath(m->filename);

	success = FileUtils::exists(m->connectionName);
	if(success)
	{
		return true;
	}

	if(!success)
	{
		spLog(Log::Info, this) << "Database " << m->connectionName << " not existent yet";
		spLog(Log::Info, this) << "Copy " << sourceDatabasefile << " to " << m->connectionName;

		success = QFile::copy(sourceDatabasefile, m->connectionName);

		if(success)
		{
			QFile f(m->connectionName);
			f.setPermissions(f.permissions() |
			                 QFile::Permission::WriteOwner | QFile::Permission::WriteUser |
			                 QFile::Permission::ReadOwner | QFile::Permission::ReadUser);

			spLog(Log::Info, this) << "DB file has been copied to " << m->connectionName;
		}

		else
		{
			spLog(Log::Error, this) << "Fatal Error: could not copy DB file to " << m->connectionName;
		}
	}

	return success;
}

void Base::transaction()
{
	db().transaction();
}

void Base::commit()
{
	db().commit();
}

void Base::rollback()
{
	db().rollback();
}

bool Base::checkAndDropTable(const QString& tablename)
{
	auto q = runQuery(QString("DROP TABLE IF EXISTS %1;").arg(tablename),
	                  QString("Cannot drop table %1").arg(tablename));

	return !hasError(q);
}

bool DB::Base::checkAndInsertColumn(const QString& tablename, const QString& column, const QString& sqltype)
{
	return checkAndInsertColumn(tablename, column, sqltype, QString());
}

bool Base::checkAndInsertColumn(const QString& tablename, const QString& column, const QString& sqltype,
                                const QString& defaultValue)
{
	const auto querytext = QString("SELECT %1 FROM %2;")
		.arg(column)
		.arg(tablename);

	auto q = runQuery(querytext, QString());
	if(hasError(q))
	{
		auto alterTable = QString("ALTER TABLE %1 ADD COLUMN %2 %3")
			.arg(tablename)
			.arg(column)
			.arg(sqltype);

		if(!defaultValue.isEmpty())
		{
			alterTable += QString(" DEFAULT %1").arg(defaultValue);
		}

		alterTable += ";";

		const auto errorString = QString("Cannot insert column %1 into %2")
			.arg(column)
			.arg(tablename);

		auto alterTableQuery = runQuery(alterTable, errorString);
		return !hasError(alterTableQuery);
	}

	return true;
}

bool Base::checkAndCreateTable(const QString& tablename, const QString& sql)
{
	const auto selectStatement = QString("SELECT * FROM %1;").arg(tablename);
	auto q = runQuery(selectStatement, QString());
	if(hasError(q))
	{
		const auto createQuery = runQuery(sql, QString("Cannot create table %1").arg(tablename));
		return !hasError(createQuery);
	}

	return true;
}
