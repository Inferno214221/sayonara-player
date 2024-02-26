/* ${CLASS_NAME}.h */
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
#ifndef SAYONARA_PLAYER_PLAYLISTINTERFACE_H
#define SAYONARA_PLAYER_PLAYLISTINTERFACE_H

#include <QString>

#include <memory>

class CustomPlaylist;
class MetaDataList;
class QStringList;

namespace Playlist
{
	class Playlist;
}

using PlaylistPtr = std::shared_ptr<::Playlist::Playlist>;

namespace Playlist
{
	class LocalPathPlaylistCreator;

	class Accessor
	{
		public:
			virtual ~Accessor() = default;

			[[nodiscard]] virtual int activeIndex() const = 0;
			[[nodiscard]] virtual PlaylistPtr activePlaylist() = 0;

			[[nodiscard]] virtual int currentIndex() const = 0;
			virtual void setCurrentIndex(int playlistIndex) = 0;

			[[nodiscard]] virtual PlaylistPtr playlist(int playlistIndex) = 0;
			[[nodiscard]] virtual PlaylistPtr playlistById(int playlistId) = 0;

			[[nodiscard]] virtual int count() const = 0;
	};

	class Creator
	{
		public:
			virtual ~Creator() = default;

			[[nodiscard]] virtual PlaylistPtr playlist(int playlistIndex) = 0;
			[[nodiscard]] virtual PlaylistPtr playlistById(int playlistId) = 0;

			[[nodiscard]] virtual QString requestNewPlaylistName(const QString& prefix = QString()) const = 0;

			virtual int
			createPlaylist(const MetaDataList& tracks, const QString& name = QString(), bool temporary = true,
			               bool isLocked = false) = 0;
			virtual int
			createPlaylist(const QStringList& pathList, const QString& name = QString(), bool temporary = true,
			               LocalPathPlaylistCreator* playlistFromPathCreator = nullptr) = 0;
			virtual int createPlaylist(const CustomPlaylist& customPlaylist) = 0;
			virtual int createEmptyPlaylist(bool override = false) = 0;
			virtual int createCommandLinePlaylist(const QStringList& pathList,
			                                      LocalPathPlaylistCreator* playlistFromPathCreator) = 0;
	};
}
#endif //SAYONARA_PLAYER_PLAYLISTINTERFACE_H
