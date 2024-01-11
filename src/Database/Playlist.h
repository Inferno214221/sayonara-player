/* DatabasePlaylist.h */

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

#ifndef DATABASEPLAYLIST_H
#define DATABASEPLAYLIST_H

#include "Database/Module.h"
#include "Utils/Playlist/Sorting.h"

#include <QList>

class CustomPlaylist;
class MetaData;
class MetaDataList;

namespace Playlist
{
	enum class StoreType :
		uint8_t
	{
		OnlyTemporary = 1,
		OnlyPermanent = 2,
		TemporaryAndPermanent = 3
	};
}

using PlaylistStoreType = ::Playlist::StoreType;
using PlaylistSortOrder = ::Playlist::SortOrder;

namespace DB
{
	class Playlist :
		private Module
	{
		public:
			Playlist(const QString& connectionName, DbId databaseId);
			~Playlist();

			int getPlaylistIdByName(const QString& name);

			CustomPlaylist getPlaylistById(int playlistId, bool getTrack);
			QList<CustomPlaylist> getAllPlaylists(PlaylistStoreType storeType, bool getTracks,
			                                      PlaylistSortOrder sortOrder = PlaylistSortOrder::NameAsc);

			int createPlaylist(const QString& playlistName, bool temporary, bool isLocked);
			bool updatePlaylistTracks(int playlistId, const MetaDataList& tracks);
			bool updatePlaylist(int playlistId, const QString& name, bool temporary, bool isLocked);
			bool renamePlaylist(int playlistId, const QString& newName);

			bool deletePlaylist(int playlistId);
			bool clearPlaylist(int playlistId);

			bool insertTrackIntoPlaylist(const MetaData& md, int playlistId, int pos);

		private:
			MetaDataList getPlaylistWithDatabaseTracks(int playlistId);
			MetaDataList getPlaylistWithNonDatabaseTracks(int playlistId);
	};
}

#endif // DATABASEPLAYLIST_H
