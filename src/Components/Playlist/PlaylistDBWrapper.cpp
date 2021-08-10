/* PlaylistDBWrapper.cpp */

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

#include "PlaylistDBWrapper.h"

#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Tagging/Tagging.h"

#include "Database/Connector.h"
#include "Database/Playlist.h"

#include <utility>

namespace
{
	MetaDataList applyTags(MetaDataList tracks)
	{
		for(auto& track : tracks)
		{
			if(track.isExtern() && Util::File::isFile(track.filepath()))
			{
				Tagging::Utils::getMetaDataOfFile(track);
			}
		}

		return tracks;
	}

	QList<CustomPlaylist> applyTagsToPlaylists(QList<CustomPlaylist> playlists)
	{
		for(auto& playlist : playlists)
		{
			auto tracks = playlist.tracks();
			auto changedTracks = applyTags(std::move(tracks));
			playlist.setTracks(std::move(changedTracks));
		}

		return playlists;
	}

	DB::Playlist* db() { return DB::Connector::instance()->playlistConnector(); }
}

namespace Playlist
{
	QList<CustomPlaylist> DBWrapper::getPlaylists(StoreType type, SortOrder sortOrder, bool getTracks)
	{
		auto playlists = db()->getAllPlaylists(type, getTracks, sortOrder);
		return getTracks
		       ? applyTagsToPlaylists(std::move(playlists))
		       : playlists;
	}

	CustomPlaylist DBWrapper::getPlaylistById(int playlistId, bool getTracks)
	{
		return db()->getPlaylistById(playlistId, getTracks);
	}

	CustomPlaylist DBWrapper::getPlaylistByName(const QString& name, bool getTracks)
	{
		const auto playlistId = db()->getPlaylistIdByName(name);
		return getPlaylistById(playlistId, getTracks);
	}

	bool DBWrapper::deletePlaylist(int id)
	{
		return db()->deletePlaylist(id);
	}

	bool DBWrapper::deletePlaylist(const QString& name)
	{
		const auto playlistId = db()->getPlaylistIdByName(name);
		return db()->deletePlaylist(playlistId);
	}

	bool DBWrapper::exists(const QString& name)
	{
		const auto playlistId = db()->getPlaylistIdByName(name);
		return (playlistId >= 0);
	}

	int DBWrapper::createPlaylist(const QString& playlistName, bool temporary)
	{
		return db()->createPlaylist(playlistName, temporary);
	}

	bool DBWrapper::updatePlaylistTracks(int playlistId, const MetaDataList& tracks)
	{
		return db()->updatePlaylistTracks(playlistId, tracks);
	}

	bool DBWrapper::updatePlaylist(int playlistId, const QString& name, bool temporary)
	{
		return db()->updatePlaylist(playlistId, name, temporary);
	}

	bool DBWrapper::renamePlaylist(int playlistId, const QString& name)
	{
		return db()->renamePlaylist(playlistId, name);
	}
}