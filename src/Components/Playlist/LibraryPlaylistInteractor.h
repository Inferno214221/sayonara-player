/* LibraryPlaylistInteractorImpl.h */
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
#ifndef SAYONARA_PLAYER_LIBRARYPLAYLISTINTERACTOR_H
#define SAYONARA_PLAYER_LIBRARYPLAYLISTINTERACTOR_H

class MetaDataList;
class QStringList;

class PlaylistAccessor;
class PlaylistCreator;
class PlayManager;

class LibraryPlaylistInteractor
{
	public:
		virtual ~LibraryPlaylistInteractor() = default;

		virtual void createPlaylist(const QStringList& paths, bool createNewPlaylist) = 0;
		virtual void createPlaylist(const MetaDataList& tracks, bool createNewPlaylist) = 0;
		virtual void append(const MetaDataList& tracks) = 0;
		virtual void insertAfterCurrentTrack(const MetaDataList& tracks) = 0;

		static LibraryPlaylistInteractor*
		create(PlaylistAccessor* playlistAccessor, PlaylistCreator* playlistCreator, PlayManager* playManager);
};

#endif //SAYONARA_PLAYER_LIBRARYPLAYLISTINTERACTOR_H
