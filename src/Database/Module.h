/* DatabaseModule.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef DATABASEMODULE_H
#define DATABASEMODULE_H

#include "Utils/Pimpl.h"

#include <QVariant>
#include <QSqlDatabase>

namespace DB
{
	class Query;
	class Module
	{
		PIMPL(Module)

		public:
			Module(const QString& connection_name, DbId db_id);
			virtual ~Module();

			QSqlDatabase	db() const;
			DbId			db_id() const;
			QString			connection_name() const;
			QString			thread_connection_name() const;

			DB::Query		run_query(const QString& query, const QString& error_text) const;
			DB::Query		run_query(const QString& query, const QPair<QString, QVariant>& bindings, const QString& error_text) const;
			DB::Query		run_query(const QString& query, const QMap<QString, QVariant>& bindings, const QString& error_text) const;

			/**
			 * @brief update
			 * @param tablename Name of table to update
			 * @param field_bindings Fields to update
			 * @param where_binding Where clause
			 * @param error_message
			 * @return
			 */
			DB::Query		update(const QString& tablename, const QMap<QString, QVariant>& field_bindings, const QPair<QString, QVariant>& where_binding, const QString& error_message);
			DB::Query		insert(const QString& tablename, const QMap<QString, QVariant>& field_bindings, const QString& error_message);
	};
}

#endif // DATABASEMODULE_H
