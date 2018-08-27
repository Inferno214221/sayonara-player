/* Query.h */

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

#ifndef Query_H
#define Query_H

#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlError>

#include "Utils/Pimpl.h"


namespace DB
{
	class Module;
	class Query :
			public QSqlQuery
	{
		PIMPL(Query)

		private:
			explicit Query(const QString& query=QString(), const QSqlDatabase& db = QSqlDatabase()) = delete;
			explicit Query(QSqlResult * result) = delete;
			explicit Query(QSqlDatabase db);

		public:
			explicit Query(const Module* module);
			Query(const QString& connection_name, DbId db_id);
			Query(const Query& other);
			Query& operator=(const Query& other);

			virtual ~Query();

			bool prepare(const QString& query);
			void bindValue(const QString & placeholder, const QVariant & val, QSql::ParamType paramType = QSql::In);
			bool exec();
			bool has_error() const;

			QString get_query_string() const;
			void show_query() const;
			void show_error(const QString& err_msg) const;

			size_t fetched_rows();
	};
}

#endif // Query_H
