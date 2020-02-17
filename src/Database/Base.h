/* AbstractDatabase.h */

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

#ifndef ABSTRACTDATABASE_H
#define ABSTRACTDATABASE_H

#include "Database/Module.h"
#include "Utils/Pimpl.h"
#include <QObject>

class QSqlDatabase;

namespace DB
{
	class Base :
		public QObject,
		public DB::Module
	{
		PIMPL(Base)

		public:
			Base(DbId databaseId, const QString& sourceDirectory, const QString& targetDirectory, const QString& filename, QObject* parent=nullptr);
			virtual ~Base();

			virtual bool closeDatabase();
			virtual bool isInitialized();

			virtual void transaction();
			virtual void commit();
			virtual void rollback();

		protected:
			virtual bool createDatabase();
			virtual bool applyFixes()=0;

            virtual bool checkAndInsertColumn(const QString& tablename, const QString& column, const QString& sqltype, const QString& default_value);
			virtual bool checkAndInsertColumn(const QString& tablename, const QString& column, const QString& sqltype);
			virtual bool checkAndCreateTable(const QString& tablename, const QString& sql_create_str);
			virtual bool checkAndDropTable(const QString& tablename);
	};
}

#endif // ABSTRACTDATABASE_H
