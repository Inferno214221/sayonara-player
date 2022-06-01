/* SmartPlaylistManager.h */
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
#ifndef SAYONARA_PLAYER_SMARTPLAYLISTMANAGER_H
#define SAYONARA_PLAYER_SMARTPLAYLISTMANAGER_H

#include "Utils/Pimpl.h"

#include <QList>
#include <QObject>

class PlaylistCreator;
class SmartPlaylist;
using SmartPlaylistPtr = std::shared_ptr<SmartPlaylist>;
class SmartPlaylistManager :
	public QObject
{
	Q_OBJECT
	PIMPL(SmartPlaylistManager)

	signals:
		void sigPlaylistsChanged();

	public:
		explicit SmartPlaylistManager(PlaylistCreator* playlistCreator);
		~SmartPlaylistManager() override;

		[[nodiscard]] SmartPlaylistPtr smartPlaylist(int index) const;
		[[nodiscard]] QList<SmartPlaylistPtr> smartPlaylists() const;

		void selectPlaylist(int index);
		void deletePlaylist(int index);
		void insertPlaylist(const SmartPlaylistPtr& smartPlaylist);
		void updatePlaylist(int index, const SmartPlaylistPtr& smartPlaylist);
};

#endif //SAYONARA_PLAYER_SMARTPLAYLISTMANAGER_H
