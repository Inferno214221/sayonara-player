/* AbstractDatabase.h */

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

#ifndef ABSTRACTDATABASE_H
#define ABSTRACTDATABASE_H

#include "Database/Module.h"
#include "Utils/Pimpl.h"
#include <QObject>

class QSqlDatabase;

namespace DB
{
	class Fixes;
	class Base :
		public QObject,
		public DB::Module
	{
		PIMPL(Base)

		public:
			Base(DbId databaseId, const QString& sourceDirectory, const QString& targetDirectory,
			     const QString& filename, Fixes* fixes, QObject* parent = nullptr);
			~Base() override;

			virtual bool closeDatabase();
			[[nodiscard]] bool isInitialized() const;
	};

	[[nodiscard]] QString createConnectionName(const QString& targetDirectory, const QString& filename);
}

#endif // ABSTRACTDATABASE_H
