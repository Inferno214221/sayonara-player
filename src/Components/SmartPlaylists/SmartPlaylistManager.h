/* SmartPlaylistManager.h */
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
#ifndef SAYONARA_PLAYER_SMARTPLAYLISTMANAGER_H
#define SAYONARA_PLAYER_SMARTPLAYLISTMANAGER_H

#include "Utils/Pimpl.h"
#include "SmartPlaylist.h"

#include <QList>
#include <QObject>

namespace Util
{
	class FileSystem;
}

struct Spid
{
	int id;

	explicit Spid(int id) :
		id {id} {}

	bool operator==(const Spid& other) const { return id == other.id; }
};

inline bool operator<(const Spid& spid1, const Spid& spid2) { return spid1.id < spid2.id; }

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
		SmartPlaylistManager(PlaylistCreator* playlistCreator, const std::shared_ptr<Util::FileSystem>& fileSystem);
		~SmartPlaylistManager() override;

		[[nodiscard]] SmartPlaylistPtr smartPlaylist(const Spid& id) const;
		[[nodiscard]] QList<SmartPlaylistPtr> smartPlaylists() const;
		SmartPlaylistPtr createAndInsert(SmartPlaylists::Type field, int id, const QList<int>& values,
		                                 bool isRandomized, LibraryId libraryId);

		void selectPlaylist(const Spid& id);
		void deletePlaylist(const Spid& id);
		void insertPlaylist(const SmartPlaylistPtr& smartPlaylist);
		void updatePlaylist(const Spid& id, const SmartPlaylistPtr& smartPlaylist);
};

#endif //SAYONARA_PLAYER_SMARTPLAYLISTMANAGER_H
