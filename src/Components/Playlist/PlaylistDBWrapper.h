/* PlaylistDBWrapper.h */

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

#ifndef PLAYLISTDB_WRAPPER_H
#define PLAYLISTDB_WRAPPER_H

#include "Database/Playlist.h"

#include <QList>

class CustomPlaylist;
namespace Playlist
{
	namespace DBWrapper
	{
		QList<CustomPlaylist>
		getPlaylists(::Playlist::StoreType type, ::Playlist::SortOrder sortOrder, bool getTracks);

		CustomPlaylist getPlaylistById(int playlistId, bool getTracks);
		CustomPlaylist getPlaylistByName(const QString& name, bool getTracks);

		int createPlaylist(const QString& playlistName, bool temporary);
		bool updatePlaylistTracks(int playlistId, const MetaDataList& tracks);
		bool updatePlaylist(int playlistId, const QString& name, bool temporary);
		bool renamePlaylist(int playlistId, const QString& name);
		bool deletePlaylist(int id);
		bool deletePlaylist(const QString& name);
		bool exists(const QString& name);
	}
}

#endif // PLAYLISTDBCONNECTOR_H
