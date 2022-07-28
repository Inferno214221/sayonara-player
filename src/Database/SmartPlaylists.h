/* SmartPlaylists.h */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_SMARTPLAYLISTS_H
#define SAYONARA_PLAYER_SMARTPLAYLISTS_H

#include "Module.h"

#include <QString>
#include <QList>

struct SmartPlaylistDatabaseEntry
{
	int id {-1};
	QString classType;
	QString attributes;
	bool isRandomized {true};
};

namespace DB
{
	class SmartPlaylists :
		private Module
	{
		public:
			SmartPlaylists(const QString& connectionName, const DbId databaseId);
			~SmartPlaylists() = default;

			QList<SmartPlaylistDatabaseEntry> getAllSmartPlaylists() const;
			int insertSmartPlaylist(const SmartPlaylistDatabaseEntry& smartPlaylistDatabaseEntry);
			bool updateSmartPlaylist(int id, const SmartPlaylistDatabaseEntry& smartPlaylistDatabaseEntry);
			bool deleteSmartPlaylist(int id);
	};
}

#endif //SAYONARA_PLAYER_SMARTPLAYLISTS_H
