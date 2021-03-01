/* PlaylistLibraryInteractor.h */
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
#ifndef SAYONARA_PLAYER_LIBRARYINTERACTOR_H
#define SAYONARA_PLAYER_LIBRARYINTERACTOR_H

#include "Utils/Pimpl.h"
#include "Utils/Playlist/PlaylistFwd.h"

#include <QObject>

class LibraryInfoAccessor;
class MetaData;

namespace Playlist
{
	class Handler;

	class LibraryInteractor :
		public QObject
	{
		Q_OBJECT
		PIMPL(LibraryInteractor)

		public:
			LibraryInteractor(Handler* playlistHandler, LibraryInfoAccessor* libraryInfoAccessor);
			~LibraryInteractor();

		private slots:
			void findTrackRequested(const MetaData& track);
			void deleteTracksReqeuested(const MetaDataList& tracks);

		private:
			void initPlaylistConnections(PlaylistPtr playlist);
	};
}

#endif //SAYONARA_PLAYER_LIBRARYINTERACTOR_H