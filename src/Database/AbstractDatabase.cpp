/* AbstractDatabase.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "Database/AbstractDatabase.h"
#include "Database/DatabaseModule.h"
#include "Database/SayonaraQuery.h"

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
	QString db_name;		// player.db
	QString db_dir;			// /home/user/.Sayonara/
	QString db_path;		// /home/user/.Sayonara/player.db
	DbId	db_id;

	bool initialized;

	Private(DbId db_id, const QString& db_dir, const QString& db_name)
	{
		this->db_id = db_id;
		this->db_dir = db_dir;
		this->db_name = db_name;
		this->db_path = Util::sayonara_path(db_name);
	}
};

Base::Base(DbId db_id, const QString& db_dir, const QString& db_name, QObject *parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(db_id, db_dir, db_name);

	if(!exists()){
		sp_log(Log::Info, this) << "Database not existent. Creating database...";
		create_db();
	}

	QSqlDatabase db = open_db();
	m->initialized = db.isOpen();

	if(!m->initialized) {
		sp_log(Log::Error, this) << "Could not open database";
	}
}

Base::~Base() {}

bool Base::is_initialized()
{
	return m->initialized;
}

bool Base::exists()
{
	return FileUtils::exists(m->db_path);
}


QSqlDatabase Base::open_db()
{
	QString connection_name = m->db_path;
	QStringList connection_names = QSqlDatabase::connectionNames();

	if(connection_names.contains(connection_name))
	{
		return QSqlDatabase::database(connection_name);
	}

	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connection_name);
	db.setDatabaseName(connection_name);

	bool success = db.open();
	if (!success) {
		sp_log(Log::Error, this) << "DatabaseConnector database cannot be opened!";
		QSqlError er = db.lastError();
		sp_log(Log::Error) << er.driverText();
		sp_log(Log::Error) << er.databaseText();
	}
	else{
		sp_log(Log::Info, this) << "Opened Database " << m->db_name;
	}

	return db;
}

bool Base::close_db()
{
	if(!QSqlDatabase::isDriverAvailable("QSQLITE")){
		return false;
	}

	QStringList connection_names = QSqlDatabase::connectionNames();
	if(!connection_names.contains(m->db_path)){
		return false;
	}

	sp_log(Log::Info) << "close database " << m->db_name << "...";

	if(db().isOpen()){
		db().close();
	}

	QSqlDatabase::removeDatabase(m->db_path);
	return true;
}


bool Base::create_db()
{
	bool success;
	QDir dir = QDir::homePath();

	QString sayonara_path = Util::sayonara_path();
	if(!FileUtils::exists(sayonara_path))
	{
		success = dir.mkdir(".Sayonara");
		if(!success) {
			sp_log(Log::Error) << "Could not create .Sayonara dir";
			return false;
		}

		else{
			sp_log(Log::Info) << "Successfully created .Sayonara dir";
		}
	}

	success = dir.cd(sayonara_path);

	//if ret is still not true we are not able to create the directory
	if(!success) {
		sp_log(Log::Error) << "Could not change to .Sayonara dir";
		return false;
	}

	QString source_db_file = Util::share_path(m->db_dir + "/" + m->db_name);

	success = FileUtils::exists(m->db_path);

	if(success) {
		return true;
	}

	if (!success) {
		sp_log(Log::Info) << "Database " << m->db_path << " not existent yet";
		sp_log(Log::Info) << "Copy " <<  source_db_file << " to " << m->db_path;

		if (QFile::copy(source_db_file, m->db_path)) {
			sp_log(Log::Info) << "DB file has been copied to " <<   m->db_path;
			success = true;
		}

		else {
			sp_log(Log::Error) << "Fatal Error: could not copy DB file to " << m->db_path;
			success = false;
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


bool Base::check_and_drop_table(const QString& tablename)
{
	Query q(db());
	QString querytext = "DROP TABLE " +  tablename + ";";
	q.prepare(querytext);

	if(!q.exec()){
		q.show_error(QString("Cannot drop table ") + tablename);
		return false;
	}

	return true;
}


bool Base::check_and_insert_column(const QString& tablename, const QString& column, const QString& sqltype, const QString& default_value)
{
	Query q(db());
	QString querytext = "SELECT " + column + " FROM " + tablename + ";";
	q.prepare(querytext);

	if(!q.exec())
	{
		Query q2 (db());
		querytext = "ALTER TABLE " + tablename + " ADD COLUMN " + column + " " + sqltype;
		if(!default_value.isEmpty()){
			querytext += " DEFAULT " + default_value;
		}

		querytext += ";";

		q2.prepare(querytext);

		if(!q2.exec())
		{
			q.show_error(QString("Cannot insert column ") + column + " into " + tablename);
			return false;
		}

		return true;
	}

	return true;
}

bool Base::check_and_create_table(const QString& tablename, const QString& sql_create_str)
{
	Query q(db());
	QString querytext = "SELECT * FROM " + tablename + ";";
	q.prepare(querytext);

	if(!q.exec())
	{
		Query q2 (db());
		q2.prepare(sql_create_str);

		if(!q2.exec()){
			q.show_error(QString("Cannot create table ") + tablename);
			return false;
		}
	}

	return true;
}

QSqlDatabase Base::db() const
{
	return QSqlDatabase::database(m->db_path);
}

DbId Base::db_id() const
{
	return m->db_id;
}

