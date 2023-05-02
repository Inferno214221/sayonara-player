/* DatabaseModule.h */

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
			Module(const QString& connectionName, DbId databaseId);
			virtual ~Module();

			[[nodiscard]] QSqlDatabase db() const;
			[[nodiscard]] DbId databaseId() const;
			[[nodiscard]] QString connectionName() const;

			DB::Query runQuery(const QString& query, const QString& errorText) const;
			DB::Query
			runQuery(const QString& query, const QPair<QString, QVariant>& bindings, const QString& errorText) const;
			DB::Query
			runQuery(const QString& query, const QMap<QString, QVariant>& bindings, const QString& errorText) const;

			DB::Query update(const QString& tablename, const QMap<QString, QVariant>& fieldBindings,
			                 const QPair<QString, QVariant>& whereBinding, const QString& errorMessage);
			DB::Query insert(const QString& tablename, const QMap<QString, QVariant>& fieldBindings,
			                 const QString& errorMessage);
	};
}

#endif // DATABASEMODULE_H
