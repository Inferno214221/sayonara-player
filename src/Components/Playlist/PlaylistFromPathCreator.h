/* PlaylistCreator.h */
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

#ifndef SAYONARA_PLAYER_PLAYLISTFROMPATHCREATOR_H
#define SAYONARA_PLAYER_PLAYLISTFROMPATHCREATOR_H

#include "Interfaces/PlaylistInterface.h"
#include "Utils/Pimpl.h"

#include <QObject>

class QStringList;

namespace Playlist
{
	class PlaylistFromPathCreator :
		public QObject
	{
		Q_OBJECT

		signals:
			void sigAllPlaylistsCreated(int firstIndex);

		public:
			static PlaylistFromPathCreator* create(PlaylistCreator* playlistCreator);

			virtual int createPlaylists(const QStringList& paths, const QString& name, bool temporary) = 0;
	};

	QString filesystemPlaylistName();
}

#endif //SAYONARA_PLAYER_PLAYLISTFROMPATHCREATOR_H
