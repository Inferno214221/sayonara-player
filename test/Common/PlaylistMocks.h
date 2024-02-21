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
#ifndef SAYONARA_PLAYER_PLAYLISTMOCKS_H
#define SAYONARA_PLAYER_PLAYLISTMOCKS_H

#include "Common/PlayManagerMock.h"

#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/PlaylistLoader.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/CustomPlaylist.h"

#include <QList>

class PlaylistLoaderMock :
	public Playlist::Loader
{
		QList<CustomPlaylist> m_playlists;

	public:
		[[nodiscard]] int getLastPlaylistIndex() const override { return -1; }

		[[nodiscard]] int getLastTrackIndex() const override { return -1; }

		[[nodiscard]] const QList<CustomPlaylist>& playlists() const override { return m_playlists; }
};

class PlaylistHandlerMock :
	public Playlist::Accessor,
	public Playlist::Creator
{
	public:
		explicit PlaylistHandlerMock(std::shared_ptr<PlayManagerMock> playManager) :
			m_playManager {std::move(playManager)} {}

		~PlaylistHandlerMock() override = default;

		[[nodiscard]] int activeIndex() const override { return m_activeIndex; }

		void setActiveIndex(const int index) { m_activeIndex = index; }

		PlaylistPtr activePlaylist() override { return m_playlists[m_activeIndex]; }

		[[nodiscard]] int currentIndex() const override { return m_currentIndex; };

		void setCurrentIndex(const int index) override { m_currentIndex = index; }

		PlaylistPtr playlist(const int index) override { return m_playlists[index]; };

		PlaylistPtr playlistById(const int id) override
		{
			const auto it = Util::Algorithm::find(m_playlists, [&](const PlaylistPtr& playlist) {
				return playlist->id() == id;
			});

			return it != m_playlists.end() ? *it : nullptr;
		}

		[[nodiscard]] int count() const override { return m_playlists.count(); }

		[[nodiscard]] QString requestNewPlaylistName(const QString& prefix) const override
		{
			static int i = 0;
			return prefix + QString::number(++i);
		}

		int createPlaylist(const MetaDataList& tracks, const QString& name, bool temporary, bool isLocked) override
		{
			auto playlist = std::make_shared<Playlist::Playlist>(m_playlists.count(), name, m_playManager.get());
			playlist->createPlaylist(tracks);
			playlist->setTemporary(temporary);
			playlist->setLocked(isLocked);
			m_playlists << playlist;

			if(m_currentIndex < 0)
			{
				setCurrentIndex(playlist->index());
			}

			if(m_activeIndex < 0)
			{
				setActiveIndex(playlist->index());
			}

			return playlist->index();
		}

		int createPlaylist(const QStringList& paths, const QString& name, bool temporary,
		                   Playlist::LocalPathPlaylistCreator* /*playlistFromPathCreator*/) override
		{
			auto tracks = MetaDataList {};
			Util::Algorithm::transform(paths, tracks, [](const auto& path) {
				return MetaData {path};
			});

			return createPlaylist(tracks, name, temporary, false);
		}

		int createPlaylist(const CustomPlaylist& customPlaylist) override
		{
			return createPlaylist(customPlaylist.tracks(), customPlaylist.name(),
			                      customPlaylist.isTemporary(), customPlaylist.isLocked());
		}

		int createEmptyPlaylist(const bool /*override*/) override
		{
			return createPlaylist(MetaDataList {}, requestNewPlaylistName({}), true, false);
		}

		int createCommandLinePlaylist(const QStringList& pathList,
		                              Playlist::LocalPathPlaylistCreator* playlistFromPathCreator) override
		{
			return createPlaylist(pathList, "command-line", true, playlistFromPathCreator);
		}

	private:
		QList<PlaylistPtr> m_playlists;
		int m_currentIndex {-1};
		int m_activeIndex {-1};
		std::shared_ptr<PlayManager> m_playManager;
};

#endif //SAYONARA_PLAYER_PLAYLISTMOCKS_H
