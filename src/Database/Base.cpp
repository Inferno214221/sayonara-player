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

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/FileUtils.h"

#include <QFile>
#include <QDir>

using DB::Base;
using DB::Query;

namespace FileUtils=::Util::File;

struct Base::Private
{
	QString sourceDirectory;
	QString targetDirectory;
	QString filename;			// player.db
	QString connectionName;	// /home/user/.Sayonara/player.db
	DbId	databaseId;

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


Base::Base(DbId databaseId, const QString& sourceDirectory, const QString& targetDirectory, const QString& filename, QObject* parent) :
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

	if(!m->initialized) {
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
	if(!QSqlDatabase::isDriverAvailable("QSQLITE")){
		return false;
	}

	QString connection_name;
	{
		QSqlDatabase database = db();
		connection_name = database.connectionName();
		QStringList connection_names = QSqlDatabase::connectionNames();
		if(!connection_names.contains(connection_name)){
			return false;
		}

		spLog(Log::Info, this) << "close database " << m->filename << " (" << connection_name << ")...";

		if(database.isOpen()){
			database.close();
		}
	}

	QSqlDatabase::removeDatabase(connection_name);

	return true;
}


bool Base::createDatabase()
{
	QDir dir = QDir::homePath();
	bool success = dir.cd(Util::sayonaraPath());

	//if ret is still not true we are not able to create the directory
	if(!success) {
		spLog(Log::Error, this) << "Could not change to .Sayonara dir";
		return false;
	}

	QString source_db_file = QDir(m->sourceDirectory).absoluteFilePath(m->filename);

	success = FileUtils::exists(m->connectionName);
	if(success) {
		return true;
	}

	if(!success)
	{
		spLog(Log::Info, this) << "Database " << m->connectionName << " not existent yet";
		spLog(Log::Info, this) << "Copy " <<  source_db_file << " to " << m->connectionName;

		success = QFile::copy(source_db_file, m->connectionName);

		if(success)
		{
			QFile f(m->connectionName);
			f.setPermissions
			(
				f.permissions() |
				QFile::Permission::WriteOwner | QFile::Permission::WriteUser |
				QFile::Permission::ReadOwner | QFile::Permission::ReadUser
			);

			spLog(Log::Info, this) << "DB file has been copied to " <<   m->connectionName;
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
	Query q(this);
	QString querytext = "DROP TABLE IF EXISTS " +  tablename + ";";
	q.prepare(querytext);

	if(!q.exec()){
		q.showError(QString("Cannot drop table ") + tablename);
		return false;
	}

	return true;
}


bool DB::Base::checkAndInsertColumn(const QString& tablename, const QString& column, const QString& sqltype)
{
    return checkAndInsertColumn(tablename, column, sqltype, QString());
}

bool Base::checkAndInsertColumn(const QString& tablename, const QString& column, const QString& sqltype, const QString& default_value)
{
	Query q(this);
	QString querytext = "SELECT " + column + " FROM " + tablename + ";";
	q.prepare(querytext);

	if(!q.exec())
	{
		Query q2 (this);
		querytext = "ALTER TABLE " + tablename + " ADD COLUMN " + column + " " + sqltype;
		if(!default_value.isEmpty()){
			querytext += " DEFAULT " + default_value;
		}

		querytext += ";";

		q2.prepare(querytext);

		if(!q2.exec())
		{
			q.showError(QString("Cannot insert column ") + column + " into " + tablename);
			return false;
		}

		return true;
	}

	return true;
}

bool Base::checkAndCreateTable(const QString& tablename, const QString& sql_create_str)
{
	Query q(this);
	QString querytext = "SELECT * FROM " + tablename + ";";
	q.prepare(querytext);

	if(!q.exec())
	{
		Query q2(this);
		q2.prepare(sql_create_str);

		if(!q2.exec()){
			q.showError(QString("Cannot create table ") + tablename);
			return false;
		}
	}

	return true;
}
