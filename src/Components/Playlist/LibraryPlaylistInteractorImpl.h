/* LibraryPlaylistInteractorImpl.h */
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
#ifndef SAYONARA_PLAYER_LIBRARYPLAYLISTINTERACTORIMPL_H
#define SAYONARA_PLAYER_LIBRARYPLAYLISTINTERACTORIMPL_H

#include "Utils/Pimpl.h"
#include "Interfaces/LibraryPlaylistInteractor.h"

namespace Playlist
{
	class Handler;
}

class PlayManager;

class LibraryPlaylistInteractorImpl :
	public LibraryPlaylistInteractor
{
	PIMPL(LibraryPlaylistInteractorImpl)

	public:
		LibraryPlaylistInteractorImpl(Playlist::Handler* playlistHandler, PlayManager* playManager);
		~LibraryPlaylistInteractorImpl() noexcept override;

		void createPlaylist(const QStringList& paths, bool createNewPlaylist) override;
		void createPlaylist(const MetaDataList& tracks, bool createNewPlaylist) override;
		void append(const MetaDataList& tracks) override;
		void insertAfterCurrentTrack(const MetaDataList& tracks) override;
};

#endif //SAYONARA_PLAYER_LIBRARYPLAYLISTINTERACTORIMPL_H
