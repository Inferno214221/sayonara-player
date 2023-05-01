/* DatabaseModule.cpp */

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

#include "Database/Query.h"
#include "Database/Module.h"
#include "Utils/Logger/Logger.h"

#include <QApplication>
#include <QThread>
#include <QSqlError>
#include <QSqlDatabase>
#include <utility>

using DB::Module;

struct Module::Private
{
	QString connectionName;
	DbId databaseId;

	Private(QString connectionName, DbId databaseId) :
		connectionName(std::move(connectionName)),
		databaseId(databaseId) {}
};

Module::Module(const QString& connectionName, DbId databaseId)
{
	m = Pimpl::make<Private>(connectionName, databaseId);
}

Module::~Module() = default;

DbId Module::databaseId() const
{
	return m->databaseId;
}

QString Module::connectionName() const
{
	return m->connectionName;
}

static void execPragma(const QSqlDatabase& db, const QString& key, const QString& value)
{
	const QString q = QString("PRAGMA %1 = %2;").arg(key).arg(value);
	db.exec(q);
}

QSqlDatabase Module::db() const
{
	if(!QSqlDatabase::isDriverAvailable("QSQLITE"))
	{
		return {};
	}

	QThread* t = QThread::currentThread();

	quint64 id = quint64(t);
	if(QApplication::instance() && (t == QApplication::instance()->thread()))
	{
		id = 0;
	}

	QString threadConnectionName = QString("%1-%2")
		.arg(m->connectionName)
		.arg(id);

	const QStringList connections = QSqlDatabase::connectionNames();
	if(connections.contains(threadConnectionName))
	{
		return QSqlDatabase::database(threadConnectionName);
	}

	spLog(Log::Info, this) << "Create new connection to " << connectionName()
	                       << " (" << threadConnectionName << ")";

	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", threadConnectionName);
	db.setDatabaseName(m->connectionName);

	if(!db.open())
	{
		QSqlError er = db.lastError();

		spLog(Log::Error, this) << "Database cannot be opened! " << m->connectionName;
		spLog(Log::Error, this) << er.driverText();
		spLog(Log::Error, this) << er.databaseText();
	}

	execPragma(db, "case_sensitive_like", "true");

	return db;
}

DB::Query Module::runQuery(const QString& query, const QString& errorText) const
{
	return runQuery(query, QMap<QString, QVariant>(), errorText);
}

DB::Query
Module::runQuery(const QString& query, const QPair<QString, QVariant>& bindings, const QString& errorText) const
{
	return runQuery(query, {{bindings.first, bindings.second}}, errorText);
}

DB::Query
Module::runQuery(const QString& query, const QMap<QString, QVariant>& bindings, const QString& errorText) const
{
	DB::Query q(this);
	q.prepare(query);

	const QList<QString> keys = bindings.keys();
	for(const QString& k: keys)
	{
		q.bindValue(k, bindings[k]);
	}

	if(!q.exec())
	{
		spLog(Log::Error, this) << "Query error to connection " << db().connectionName();
		q.showError(errorText);
	}

	return q;
}

DB::Query
Module::insert(const QString& tablename, const QMap<QString, QVariant>& fieldBindings, const QString& errorMessage)
{
	const QList<QString> fieldNames = fieldBindings.keys();
	const QString fields = fieldNames.join(", ");
	const QString bindings = ":" + fieldNames.join(", :");

	QString query = "INSERT INTO " + tablename + " ";
	query += "(" + fields + ") ";
	query += "VALUES (" + bindings + ");";

	DB::Query q(this);
	q.prepare(query);

	for(const QString& field: fieldNames)
	{
		q.bindValue(":" + field, fieldBindings[field]);
	}

	if(!q.exec())
	{
		spLog(Log::Error, this) << "Query error to connection " << db().connectionName();
		q.showError(errorMessage);
	}

	return q;
}

DB::Query Module::update(const QString& tablename, const QMap<QString, QVariant>& fieldBindings,
                         const QPair<QString, QVariant>& whereBinding, const QString& errorMessage)
{
	const QList<QString> fieldNames = fieldBindings.keys();

	QStringList updateCommands;
	for(const QString& field: fieldNames)
	{
		updateCommands << field + " = :" + field;
	}

	QString query = "UPDATE " + tablename + " SET ";
	query += updateCommands.join(", ");
	query += " WHERE ";
	query += whereBinding.first + " = :W" + whereBinding.first;
	query += ";";

	DB::Query q(this);
	q.prepare(query);

	for(const QString& field: fieldNames)
	{
		q.bindValue(":" + field, fieldBindings[field]);
	}

	q.bindValue(":W" + whereBinding.first, whereBinding.second);

	if(!q.exec() || q.numRowsAffected() == 0)
	{
		spLog(Log::Error, this) << "Query error to connection " << db().connectionName();
		q.setError(true);
		q.showError(errorMessage);
	}

	return q;
}
