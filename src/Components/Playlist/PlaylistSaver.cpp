/* PlaylistSaver.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "PlaylistSaver.h"
#include "Playlist.h"
#include "PlaylistModifiers.h"

#include "Utils/Settings/Settings.h"

namespace Playlist
{
	void saveCurrentPlaylists(const QList<PlaylistPtr>& playlists)
	{
		SetSetting(Set::PL_LastPlaylist, -1);
		SetSetting(Set::PL_LastTrack, -1);

		auto ids = QList<int> {};

		for(const auto& playlist: playlists)
		{
			ids << playlist->id();

			const auto isTemporary = playlist->isTemporary();
			const auto isActive = (playlist->currentTrackIndex() >= 0);

			if(isActive)
			{
				SetSetting(Set::PL_LastPlaylist, playlist->id());
				SetSetting(Set::PL_LastTrack, currentTrackWithoutDisabled(*playlist));
			}

			if(isTemporary)
			{
				const auto saveEntireSession = GetSetting(Set::PL_LoadRecentPlaylists);
				const auto saveUnsavedPlaylist = (GetSetting(Set::PL_LoadTemporaryPlaylists) && playlist->wasChanged());
				const auto isCurrentlyPlaying = (GetSetting(Set::PL_LoadLastTrack) && isActive);

				if(saveEntireSession || saveUnsavedPlaylist || isCurrentlyPlaying)
				{
					playlist->save();
				}

				else
				{
					playlist->deletePlaylist();
				}
			}
		}

		SetSetting(Set::PL_RecentPlaylists, ids);
	}
}