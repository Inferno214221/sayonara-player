/* DatabaseModule.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "Database/Query.h"
#include "Database/Module.h"
#include "Utils/Logger/Logger.h"

#include <QApplication>
#include <QThread>
#include <QSqlError>
#include <QSqlDatabase>

using DB::Module;

struct Module::Private
{
	QString connection_name;
	DbId	db_id;

	Private(const QString& connection_name, DbId db_id) :
		connection_name(connection_name),
		db_id(db_id)
	{}
};

Module::Module(const QString& connection_name, DbId db_id)
{
	m = Pimpl::make<Private>(connection_name, db_id);
}

Module::~Module() = default;

DbId Module::db_id() const
{
	return m->db_id;
}

QString Module::connection_name() const
{
	return m->connection_name;
}

QSqlDatabase Module::db() const
{
	if(!QSqlDatabase::isDriverAvailable("QSQLITE")){
		return QSqlDatabase();
	}

	QThread* t = QThread::currentThread();

	quint64 id = quint64(t);
	if(t == QApplication::instance()->thread()) {
		id = 0;
	}

	QString thread_connection_name = QString("%1-%2")
									.arg(m->connection_name)
									.arg(id);

	const QStringList connections = QSqlDatabase::connectionNames();
	if(connections.contains(thread_connection_name))
	{
		return QSqlDatabase::database(thread_connection_name);
	}

	sp_log(Log::Info, this) << "Create new connection to " << connection_name()
							<< " (" << thread_connection_name << ")";

	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", thread_connection_name);
	db.setDatabaseName(m->connection_name);

	if(!db.open())
	{
		QSqlError er = db.lastError();

		sp_log(Log::Error, this) << "Database cannot be opened! " << m->connection_name;
		sp_log(Log::Error, this) << er.driverText();
		sp_log(Log::Error, this) << er.databaseText();
	}

	return db;
}

DB::Query Module::run_query(const QString& query, const QString& error_text) const
{
	return run_query(query, QMap<QString, QVariant>(), error_text);
}

DB::Query Module::run_query(const QString& query, const QPair<QString, QVariant>& bindings, const QString& error_text) const
{
	return run_query(query, {{bindings.first, bindings.second}}, error_text);
}

DB::Query Module::run_query(const QString& query, const QMap<QString, QVariant>& bindings, const QString& error_text) const
{
	DB::Query q(this);
	q.prepare(query);

	const QList<QString> keys = bindings.keys();
	for(const QString& k : keys)
	{
		q.bindValue(k, bindings[k]);
	}

	if(!q.exec())
	{
		sp_log(Log::Error, this) << "Query error to connection " << db().connectionName();
		q.show_error(error_text);
	}

	return q;
}

DB::Query Module::insert(const QString& tablename, const QMap<QString, QVariant>& field_bindings, const QString& error_message)
{
	QList<QString> field_names = field_bindings.keys();
	QString fields = field_names.join(", ");
	QString bindings = ":" + field_names.join(", :");

	QString query = "INSERT INTO " + tablename + " ";
	query += "(" + fields + ") ";
	query += "VALUES (" + bindings + ");";

	DB::Query q(this);
	q.prepare(query);

	for(const QString& field : field_names)
	{
		q.bindValue(":" + field, field_bindings[field]);
	}

	if(!q.exec())
	{
		sp_log(Log::Error, this) << "Query error to connection " << db().connectionName();
		q.show_error(error_message);
	}

	return q;
}


DB::Query Module::update(const QString& tablename, const QMap<QString, QVariant>& field_bindings, const QPair<QString, QVariant>& where_binding, const QString& error_message)
{
	const QList<QString> field_names = field_bindings.keys();

	QStringList update_commands;
	for(const QString& field : field_names)
	{
		update_commands << field + " = :" + field;
	}

	QString query = "UPDATE " + tablename + " SET ";
	query += update_commands.join(", ");
	query += " WHERE ";
	query += where_binding.first + " = :W" + where_binding.first;
	query += ";";

	DB::Query q(this);
	q.prepare(query);

	for(const QString& field : field_names)
	{
		q.bindValue(":" + field, field_bindings[field]);
	}

	q.bindValue(":W" + where_binding.first, where_binding.second);

	if(!q.exec() || q.numRowsAffected() == 0)
	{
		sp_log(Log::Error, this) << "Query error to connection " << db().connectionName();
		q.set_error(true);
		q.show_error(error_message);
	}

	return q;
}
