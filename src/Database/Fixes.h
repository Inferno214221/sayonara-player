/* Fixes.h */
/*
 * Copyright (C) 2011-2023 Michael Lugmair
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

#ifndef SAYONARA_PLAYER_FIXES_H
#define SAYONARA_PLAYER_FIXES_H

#include "Module.h"
#include "Utils/Pimpl.h"

#include <QSqlDatabase>

namespace DB
{
	class Fixes
	{
		PIMPL(Fixes)

		public:
			Fixes(const QString& connectionName, DbId databaseId);
			virtual ~Fixes() noexcept;

			Fixes(const Fixes& other) = delete;
			Fixes(Fixes&& other) = delete;
			Fixes& operator=(const Fixes& other) = delete;
			Fixes& operator=(Fixes&& other) = delete;

			bool checkAndInsertColumn(const QString& tablename, const QString& column, const QString& sqltype,
			                          const QString& defaultValue);
			bool checkAndInsertColumn(const QString& tablename, const QString& column, const QString& sqltype);
			bool checkAndCreateTable(const QString& tablename, const QString& sql);
			bool checkAndDropTable(const QString& tablename);

			virtual void applyFixes() = 0;
	};
} // DB

#endif //SAYONARA_PLAYER_FIXES_H
