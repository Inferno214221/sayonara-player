/* StandardConnectorFixes.h */
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

#ifndef SAYONARA_PLAYER_STANDARDCONNECTORFIXES_H
#define SAYONARA_PLAYER_STANDARDCONNECTORFIXES_H

#include "Utils/Pimpl.h"
#include "Database/Fixes.h"

namespace DB
{
	class StandardConnectorFixes :
		public Fixes
	{
		PIMPL(StandardConnectorFixes)

		public:
			StandardConnectorFixes(const QString& connectionName, DbId databaseId);
			~StandardConnectorFixes() noexcept override;

			StandardConnectorFixes(const StandardConnectorFixes& other) = delete;
			StandardConnectorFixes(StandardConnectorFixes&& other) = delete;
			StandardConnectorFixes& operator=(const StandardConnectorFixes& other) = delete;
			StandardConnectorFixes& operator=(StandardConnectorFixes&& other) = delete;

			void applyFixes() override;

			static int latestDatabaseVersion();

		private:
			[[nodiscard]] bool isUpdateNeeded() const;
			[[nodiscard]] int currentDatabaseVersion() const;
	};
}

#endif //SAYONARA_PLAYER_STANDARDCONNECTORFIXES_H
