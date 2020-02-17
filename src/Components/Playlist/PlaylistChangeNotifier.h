/* PlaylistChangeNotifier.h */

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



#ifndef PLAYLISTCHANGENOTIFIER_H
#define PLAYLISTCHANGENOTIFIER_H

#include "Utils/Singleton.h"
#include <QObject>

class PlaylistChangeNotifier : public QObject
{
	Q_OBJECT
	SINGLETON(PlaylistChangeNotifier)

	signals:
		void sigPlaylistRenamed(int id, const QString& oldName, const QString& newName);
		void sigPlaylistAdded(int id, const QString& name);
		void sigPlaylistDeleted(int id);

	public:
		void deletePlaylist(int id);
		void addPlaylist(int id, const QString& name);
		void renamePlaylist(int id, const QString& oldName, const QString& newName);
};

#endif // PLAYLISTCHANGENOTIFIER_H
