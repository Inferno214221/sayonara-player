/* DBusSessionManager.h */
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

#ifndef SAYONARA_PLAYER_DBUSSESSIONMANAGER_H
#define SAYONARA_PLAYER_DBUSSESSIONMANAGER_H

#include "Components/PlayManager/PlayManager.h"
#include "DBus/SessionManager.h"
#include "Utils/Pimpl.h"

#include <QObject>

class PlayManager;

namespace Dbus
{
	class SessionManager :
		public QObject
	{
		Q_OBJECT
		PIMPL(SessionManager)

		public:
			SessionManager(PlayManager* playManager);
			~SessionManager() noexcept override;

			SessionManager(const SessionManager& other) = delete;
			SessionManager(SessionManager&& other) = delete;
			SessionManager& operator=(const SessionManager& other) = delete;
			SessionManager& operator=(SessionManager&& other) = delete;

		private slots:
			void playstateChanged(PlayState playState);
			void inhibitSettingChanged();

		private: // NOLINT(readability-redundant-access-specifiers)
			void inhibit();
			void uninhibit();
	};
}

#endif //SAYONARA_PLAYER_DBUSSESSIONMANAGER_H
