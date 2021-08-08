/* PlaylistSaver.cpp */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "Utils/Settings/Settings.h"

void Playlist::saveCurrentPlaylists(const QList<PlaylistPtr>& playlists)
{
	SetSetting(Set::PL_LastPlaylist, -1);
	SetSetting(Set::PL_LastTrack, -1);

	for(auto playlist : playlists)
	{
		const auto isTemporary = playlist->isTemporary();
		const auto isActive = (playlist->currentTrackIndex() >= 0);

		if(isActive)
		{
			SetSetting(Set::PL_LastPlaylist, playlist->id());
			SetSetting(Set::PL_LastTrack, playlist->currentTrackWithoutDisabled());
		}

		if(GetSetting(Set::PL_LoadTemporaryPlaylists))
		{
			const auto wasChanged = playlist->wasChanged();
			if(isTemporary && wasChanged)
			{
				playlist->save();
			}
		}

		else // delete temporary playlists
		{
			if(isTemporary)
			{
				if(GetSetting(Set::PL_LoadLastTrack) && isActive)
				{
					playlist->save();
				}

				else
				{
					playlist->deletePlaylist();
				}
			}
		}
	}
}