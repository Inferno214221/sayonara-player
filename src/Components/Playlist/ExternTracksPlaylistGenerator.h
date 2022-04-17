/* ExternTracksPlaylistGenerator.h */

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

#ifndef EXTERNTRACKSPLAYLISTGENERATOR_H
#define EXTERNTRACKSPLAYLISTGENERATOR_H

#include "Utils/Pimpl.h"
#include "Utils/Singleton.h"
#include "Utils/Playlist/PlaylistFwd.h"

#include <QObject>

class QStringList;
class MetaDataList;
class PlaylistCreator;

class ExternTracksPlaylistGenerator :
	public QObject
{
	Q_OBJECT
	PIMPL(ExternTracksPlaylistGenerator)

	public:
		ExternTracksPlaylistGenerator(PlaylistCreator* playlistCreator, const PlaylistPtr& playlist);
		~ExternTracksPlaylistGenerator() override;

		void addPaths(const QStringList& paths);
		void insertPaths(const QStringList& paths, int targetRowIndex);

	signals:
		void sigFinished();

	private slots:
		void scanFiles(const QStringList& paths);
		void filesScanned();
};

#endif // EXTERNTRACKSPLAYLISTGENERATOR_H
