/* Fixes.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "Fixes.h"
#include "Module.h"
#include "Query.h"
#include "Settings.h"
#include "Utils/Logger/Logger.h"

#include "Utils/Utils.h"

#include <QSqlQuery>

namespace DB
{
	struct Fixes::Private
	{
		QString connectionName;
		DbId databaseId;

		Private(QString connectionName, const DbId databaseId) :
			connectionName {std::move(connectionName)},
			databaseId {databaseId} {}
	};

	Fixes::Fixes(const QString& connectionName, const DbId databaseId) :
		m {Pimpl::make<Private>(connectionName, databaseId)} {}

	Fixes::~Fixes() noexcept = default;

	bool Fixes::checkAndDropTable(const QString& tablename)
	{
		auto module = Module(m->connectionName, m->databaseId);
		auto q = module.runQuery(QString("DROP TABLE IF EXISTS %1;").arg(tablename),
		                         QString("Cannot drop table %1").arg(tablename));

		return !hasError(q);
	}

	bool Fixes::checkAndInsertColumn(const QString& tablename, const QString& column, const QString& sqltype)
	{
		return checkAndInsertColumn(tablename, column, sqltype, QString());
	}

	bool Fixes::checkAndInsertColumn(const QString& tablename, const QString& column, const QString& sqltype,
	                                 const QString& defaultValue)
	{
		auto module = Module {m->connectionName, m->databaseId};
		const auto querytext = QString("SELECT %1 FROM %2;")
			.arg(column)
			.arg(tablename);

		auto q = module.runQuery(querytext, QString());
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

			auto alterTableQuery = module.runQuery(alterTable, errorString);
			return !hasError(alterTableQuery);
		}

		return true;
	}

	bool Fixes::checkAndCreateTable(const QString& tablename, const QString& sql)
	{
		auto module = Module {m->connectionName, m->databaseId};

		const auto selectStatement = QString("SELECT * FROM %1;").arg(tablename);
		auto q = module.runQuery(selectStatement, QString());
		if(hasError(q))
		{
			const auto createQuery = module.runQuery(sql, QString("Cannot create table %1").arg(tablename));
			return !hasError(createQuery);
		}

		return true;
	}

	bool Fixes::removeColumn(const QString& tablename, const QString& column)
	{
		auto module = Module {m->connectionName, m->databaseId};

		const auto str = QString("ALTER TABLE %1 DROP COLUMN %2;")
			.arg(tablename)
			.arg(column);

		const auto q = module.runQuery(str, QString("Cannot remove column %1 from %2").arg(column).arg(tablename));
		return !hasError(q);
	}
} // DB
