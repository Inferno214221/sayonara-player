/* Query.cpp */

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

#include <QMap>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QVariant>

#include "Utils/Logger/Logger.h"

namespace
{
	QString trimInside(const QString& originalString, const QString& pattern, const QString& replacement)
	{
		auto result = originalString;
		while(result.contains(pattern))
		{
			result.replace(pattern, replacement);
		}
		return result;
	}

	QString formatQueryString(QString queryString)
	{
		queryString.prepend("\n");
		queryString.replace("SELECT ", "SELECT\n", Qt::CaseInsensitive);
		queryString.replace("FROM", "\nFROM", Qt::CaseInsensitive);
		queryString.replace(",", ",\n", Qt::CaseInsensitive);
		queryString.replace("INNER JOIN", "\nINNER JOIN", Qt::CaseInsensitive);
		queryString.replace("LEFT OUTER JOIN", "\nLEFT OUTER JOIN", Qt::CaseInsensitive);
		queryString.replace("UNION", "\nUNION", Qt::CaseInsensitive);
		queryString.replace("GROUP BY", "\nGROUP BY", Qt::CaseInsensitive);
		queryString.replace("ORDER BY", "\nORDER BY", Qt::CaseInsensitive);
		queryString.replace("WHERE", "\nWHERE", Qt::CaseInsensitive);
		queryString.replace("(", "\n(\n");
		queryString.replace(")", "\n)\n");

		auto idx = queryString.indexOf("(");
		while(idx >= 0)
		{
			const auto closingIndex = queryString.indexOf(")", idx);

			auto newlineIndex = queryString.indexOf("\n", idx);
			while(newlineIndex > 0 && newlineIndex < closingIndex)
			{
				queryString.insert(newlineIndex + 1, '\t');
				newlineIndex = queryString.indexOf("\n", newlineIndex + 2);
			}

			idx = queryString.indexOf("(", closingIndex);
		}

		queryString = trimInside(queryString, "\n ", "\n");
		queryString = trimInside(queryString, ", ", ",");
		queryString = trimInside(queryString, " ,", ",");
		queryString = trimInside(queryString, "  ", " ");
		queryString = trimInside(queryString, "\n\n", "\n");

		return queryString;
	}

	QString extractSqlString(const QSqlQuery& q)
	{
		const auto boundValues = q.boundValues();
		auto sql = q.executedQuery();

		for(auto it = boundValues.begin(); it != boundValues.end(); it++)
		{
			sql.replace(it.key(), it.value().toString());
		}

		return formatQueryString(sql);
	}
}

namespace DB
{
	bool hasError(const QSqlQuery& q)
	{
		return q.lastError().isValid() ||
		       (q.lastError().type() != QSqlError::NoError);
	}

	bool wasUpdateSuccessful(const QSqlQuery& q)
	{
		return !hasError(q) && (q.numRowsAffected() > 0);
	}

	void showError(const QSqlQuery& q, const QString& errorMessage)
	{
		constexpr const auto* ClassName = "SqlQuery";
		spLog(Log::Error, ClassName) << "SQL ERROR: " << errorMessage
		                             << ": " << static_cast<int>(q.lastError().type());

		const auto sqlError = q.lastError();
		if(!sqlError.text().isEmpty())
		{
			spLog(Log::Error, ClassName) << sqlError.text();
		}

		if(!sqlError.driverText().isEmpty())
		{
			spLog(Log::Error, ClassName) << sqlError.driverText();
		}

		if(!sqlError.databaseText().isEmpty())
		{
			spLog(Log::Error, ClassName) << sqlError.databaseText();
		}

		spLog(Log::Error, ClassName) << extractSqlString(q);
	}
}