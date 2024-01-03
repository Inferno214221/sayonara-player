/* SmartPlaylistCreator.h */
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
#ifndef SAYONARA_PLAYER_SMARTPLAYLISTCREATOR_H
#define SAYONARA_PLAYER_SMARTPLAYLISTCREATOR_H

#include "Utils/Pimpl.h"
#include "SmartPlaylist.h"

#include <QString>
#include <QList>

class SmartPlaylist;
struct SmartPlaylistDatabaseEntry;

namespace Util
{
	class FileSystem;
}

namespace SmartPlaylists
{
	std::shared_ptr<SmartPlaylist>
	create(const SmartPlaylistDatabaseEntry& entry, const std::shared_ptr<Util::FileSystem>& fileSystem);
	std::shared_ptr<SmartPlaylist>
	createFromType(SmartPlaylists::Type field, int id, const QList<int>& values, bool isRandomized,
	               LibraryId libraryId, const std::shared_ptr<Util::FileSystem>& fileSystem);
};

#endif //SAYONARA_PLAYER_SMARTPLAYLISTCREATOR_H
