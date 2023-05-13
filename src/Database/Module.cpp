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

#include <optional>
#include <utility>

using DB::Module;

namespace
{
	void execPragma(const QSqlDatabase& db, const QString& key, const QString& value)
	{
		const auto q = QString("PRAGMA %1 = %2;").arg(key).arg(value);
		db.exec(q);
	}

	quint64 getCurrentThreadId()
	{
		auto* thread = QThread::currentThread();
		return QApplication::instance() && (thread == QApplication::instance()->thread())
		       ? 0
		       : reinterpret_cast<intptr_t>(thread);
	}

	QString getThreadedConnectionName(const QString& baseConnectioName)
	{
		const auto threadId = getCurrentThreadId();
		return QString("%1-%2")
			.arg(baseConnectioName)
			.arg(threadId);
	}

	QSqlDatabase createNewDatabase(const QString& baseConnectioName)
	{
		constexpr const auto* instanceName = "DB::Module";

		const auto threadConnectionName = getThreadedConnectionName(baseConnectioName);
		spLog(Log::Info, instanceName) << "Create new connection to " << baseConnectioName
		                               << " (" << threadConnectionName << ")";

		auto db = QSqlDatabase::addDatabase("QSQLITE", threadConnectionName);
		db.setDatabaseName(baseConnectioName);

		if(!db.open())
		{
			const auto error = db.lastError();

			spLog(Log::Error, instanceName) << "Database cannot be opened! " << baseConnectioName;
			spLog(Log::Error, instanceName) << error.driverText();
			spLog(Log::Error, instanceName) << error.databaseText();
		}

		execPragma(db, "case_sensitive_like", "true");

		return db;
	}

	QString createFieldsString(const QStringList& fieldNames)
	{
		return fieldNames.join(", ");
	}

	QString createInsertPlaceholderString(const QStringList& fieldNames)
	{
		return QString(":%1")
			.arg(fieldNames.join(", :"));
	}

	QSqlQuery
	createInsertQuery(const QSqlDatabase& db, const QString& tablename, const QMap<QString, QVariant>& bindings)
	{
		const auto fieldNames = bindings.keys();
		const auto queryString = QString("INSERT INTO %1 (%2) VALUES (%3);")
			.arg(tablename)
			.arg(createFieldsString(fieldNames))
			.arg(createInsertPlaceholderString(fieldNames));

		auto query = QSqlQuery(db);
		query.prepare(queryString);

		for(const auto& fieldName: fieldNames)
		{
			query.bindValue(":" + fieldName, bindings[fieldName]);
		}

		return query;
	}

	QString createUpdatePlaceholderString(const QStringList& fieldNames)
	{
		QStringList updateCommands;
		for(const auto& field: fieldNames)
		{
			updateCommands << QString("%1 = :%1")
				.arg(field);
		}

		return updateCommands.join(", ");
	}

	QSqlQuery
	createUpdateQuery(const QSqlDatabase& db, const QString& tablename, const QMap<QString, QVariant>& fieldBindings,
	                  const QPair<QString, QVariant>& whereBinding)
	{
		const auto fieldNames = fieldBindings.keys();
		const auto queryString = QString("UPDATE %1 SET %2 WHERE %3 = :W%3;")
			.arg(tablename)
			.arg(createUpdatePlaceholderString(fieldNames))
			.arg(whereBinding.first);

		auto query = QSqlQuery(db);
		query.prepare(queryString);

		for(const auto& field: fieldNames)
		{
			query.bindValue(":" + field, fieldBindings[field]);
		}

		query.bindValue(":W" + whereBinding.first, whereBinding.second);

		return query;
	}

	QSqlQuery
	createGenericQuery(const QSqlDatabase& db, const QString& queryText, const QMap<QString, QVariant>& bindings)
	{
		auto query = QSqlQuery(db);
		query.prepare(queryText);

		const auto keys = bindings.keys();
		for(const auto& key: keys)
		{
			query.bindValue(key, bindings[key]);
		}

		return query;
	}
}

struct Module::Private
{
	QString connectionName;
	DbId databaseId;

	Private(QString connectionName, const DbId databaseId) :
		connectionName(std::move(connectionName)),
		databaseId(databaseId) {}
};

Module::Module(const QString& connectionName, const DbId databaseId) :
	m {Pimpl::make<Private>(connectionName, databaseId)} {}

Module::~Module() = default;

DbId Module::databaseId() const { return m->databaseId; }

QString Module::connectionName() const { return m->connectionName; }

QSqlDatabase Module::db() const
{
	if(!QSqlDatabase::isDriverAvailable("QSQLITE"))
	{
		return {};
	}

	const auto threadedConnectionName = getThreadedConnectionName(m->connectionName);
	return QSqlDatabase::connectionNames().contains(threadedConnectionName)
	       ? QSqlDatabase::database(threadedConnectionName)
	       : createNewDatabase(m->connectionName);
}

QSqlQuery Module::runQuery(const QString& query, const QString& errorText) const
{
	return runQuery(query, QMap<QString, QVariant>(), errorText);
}

QSqlQuery
Module::runQuery(const QString& query, const QPair<QString, QVariant>& bindings, const QString& errorText) const
{
	return runQuery(query, {{bindings.first, bindings.second}}, errorText);
}

QSqlQuery
Module::runQuery(const QString& queryText, const QMap<QString, QVariant>& bindings, const QString& errorText) const
{
	auto query = createGenericQuery(db(), queryText, bindings);
	if(!query.exec())
	{
		spLog(Log::Error, this) << "Query error to connection " << db().connectionName();
		showError(query, errorText);
	}

	return query;
}

QSqlQuery
Module::insert(const QString& tablename, const QMap<QString, QVariant>& fieldBindings, const QString& errorMessage)
{
	auto query = createInsertQuery(db(), tablename, fieldBindings);
	if(!query.exec())
	{
		spLog(Log::Error, this) << "Query error to connection " << db().connectionName();
		showError(query, errorMessage);
	}

	return query;
}

QSqlQuery Module::update(const QString& tablename, const QMap<QString, QVariant>& fieldBindings,
                         const QPair<QString, QVariant>& whereBinding, const QString& errorMessage)
{
	auto query = createUpdateQuery(db(), tablename, fieldBindings, whereBinding);
	if(!query.exec())
	{
		spLog(Log::Error, this) << "Query error to connection " << db().connectionName();
		showError(query, errorMessage);
	}

	else if(query.numRowsAffected() == 0)
	{
		spLog(Log::Debug, this) << errorMessage << ": No rows affected.";
	}

	return query;
}
