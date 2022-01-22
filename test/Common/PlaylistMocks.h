/* ${CLASS_NAME}.h */
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
#ifndef SAYONARA_PLAYER_PLAYLISTMOCKS_H
#define SAYONARA_PLAYER_PLAYLISTMOCKS_H

#include "Utils/Playlist/CustomPlaylist.h"
#include "Components/Playlist/PlaylistLoader.h"
#include "Components/Playlist/PlaylistHandler.h"

#include <QList>

class PlaylistLoaderMock : public Playlist::Loader
{
		QList<CustomPlaylist> m_playlists;

	public:
		int getLastPlaylistIndex() const override
		{
			return -1;
		}

		int getLastTrackIndex() const override
		{
			return -1;
		}

		const QList<CustomPlaylist>& playlists() const override
		{
			return m_playlists;
		}
};

using PlaylistHandlerMock = Playlist::Handler;

#endif //SAYONARA_PLAYER_PLAYLISTMOCKS_H
