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
#include "Utils/Logger/Logger.h"

#include <cassert>

namespace Playlist
{
	void saveCurrentPlaylists(const QList<PlaylistPtr>& playlists)
	{
		SetSetting(Set::PL_LastPlaylist, -1);
		SetSetting(Set::PL_LastTrack, -1);

		const auto saveEntireSession = GetSetting(Set::PL_LoadRecentPlaylists);

		auto ids = QList<int> {};

		for(const auto& playlist: playlists)
		{
			if(playlist->id() < 0)
			{
				spLog(Log::Warning, "PlaylistSaver") << "trying to save invalid playlist: " << playlist->name();
				continue;
			}

			const auto isTemporary = playlist->isTemporary();
			const auto isActive = (playlist->currentTrackIndex() >= 0);
			const auto isCurrentlyPlaying = (GetSetting(Set::PL_LoadLastTrack) && isActive);

			if(isActive)
			{
				SetSetting(Set::PL_LastPlaylist, playlist->id());
				SetSetting(Set::PL_LastTrack, currentTrackWithoutDisabled(*playlist));
			}

			if(isTemporary)
			{
				const auto saveTemporary = GetSetting(Set::PL_LoadTemporaryPlaylists);
				if(saveEntireSession || saveTemporary || isCurrentlyPlaying)
				{
					ids << playlist->id();
					playlist->save();
				}

				else
				{
					playlist->deletePlaylist();
				}
			}

			else
			{
				if(playlist->wasChanged())
				{
					playlist->save();
				}

				if(saveEntireSession || GetSetting(Set::PL_LoadSavedPlaylists) || isCurrentlyPlaying)
				{
					ids << playlist->id();
				}
			}
		}

		SetSetting(Set::PL_RecentPlaylists, ids);
	}
}