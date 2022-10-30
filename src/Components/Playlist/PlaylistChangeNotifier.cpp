/* PlaylistChangeNotifier.cpp */

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

#include "PlaylistChangeNotifier.h"

namespace Playlist
{
	ChangeNotifier::ChangeNotifier() = default;

	ChangeNotifier::~ChangeNotifier() = default;

	void ChangeNotifier::deletePlaylist(int id)
	{
		emit sigPlaylistDeleted(id);
	}

	void ChangeNotifier::addPlaylist(int id, const QString& name)
	{
		emit sigPlaylistAdded(id, name);
	}

	void ChangeNotifier::renamePlaylist(int id, const QString& old_name, const QString& new_name)
	{
		emit sigPlaylistRenamed(id, old_name, new_name);
	}
}