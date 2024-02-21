/* LibraryPlaylistInteractorImpl.cpp */
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

#include "LibraryPlaylistInteractor.h"

#include "PlayManager/PlayManager.h"
#include "Playlist.h"
#include "PlaylistHandler.h"
#include "PlaylistModifiers.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

namespace
{
	template<typename T>
	void createPlaylistFromList(const T& tracks, bool createNewPlaylist, Playlist::Creator* playlistCreator)
	{
		const auto name = (createNewPlaylist)
		                  ? playlistCreator->requestNewPlaylistName()
		                  : QString();

		playlistCreator->createPlaylist(tracks, name);
	}

	int zeroOrRandomValue(const bool shuffle, const int count)
	{
		return (shuffle && GetSetting(Set::PL_StartAtRandomTrackOnShuffle))
		       ? Util::randomNumber(0, count - 1)
		       : 0;
	}

	void applyPlaylistActionAfterDoubleClick(PlayManager* playManager, Playlist::Accessor* playlistAccessor)
	{
		if(GetSetting(Set::Lib_DC_DoNothing))
		{
			return;
		}

		auto newTrack = -1;

		const auto currentIndex = playlistAccessor->currentIndex();
		const auto currentPlaylist = playlistAccessor->playlist(currentIndex);
		const auto playlistMode = GetSetting(Set::PL_Mode);
		const auto shuffle = PlaylistMode::isActiveAndEnabled(playlistMode.shuffle());
		const auto count = currentPlaylist->count();

		if(GetSetting(Set::Lib_DC_PlayIfStopped))
		{
			if(currentPlaylist && (playManager->playstate() != PlayState::Playing))
			{
				newTrack = zeroOrRandomValue(shuffle, count);
			}
		}

		else if(GetSetting(Set::Lib_DC_PlayImmediately))
		{
			const auto plm = GetSetting(Set::PL_Mode);
			if(currentPlaylist && (plm.append() == ::Playlist::Mode::State::Off))
			{
				newTrack = zeroOrRandomValue(shuffle, count);
			}
		}

		if(newTrack >= 0)
		{
			currentPlaylist->changeTrack(newTrack);
		}
	}

	class LibraryPlaylistInteractorImpl :
		public LibraryPlaylistInteractor
	{
		public:

			LibraryPlaylistInteractorImpl(Playlist::Accessor* playlistAccessor, Playlist::Creator* playlistCreator,
			                              PlayManager* playManager) :
				m_playlistAccessor {playlistAccessor},
				m_playlistCreator {playlistCreator},
				m_playManager {playManager} {}

			~LibraryPlaylistInteractorImpl() noexcept override = default;

			void createPlaylist(const QStringList& tracks, bool createNewPlaylist) override
			{
				createPlaylistFromList(tracks, createNewPlaylist, m_playlistCreator);
				applyPlaylistActionAfterDoubleClick(m_playManager, m_playlistAccessor);
			}

			void createPlaylist(const MetaDataList& tracks, bool createNewPlaylist) override
			{
				createPlaylistFromList(tracks, createNewPlaylist, m_playlistCreator);
				applyPlaylistActionAfterDoubleClick(m_playManager, m_playlistAccessor);
			}

			void append(const MetaDataList& tracks) override
			{
				Playlist::appendTracks(*m_playlistAccessor->activePlaylist(), tracks, Playlist::Reason::Library);
			}

			void insertAfterCurrentTrack(const MetaDataList& tracks) override
			{
				auto playlist = m_playlistAccessor->activePlaylist();
				Playlist::insertTracks(*playlist, tracks, playlist->currentTrackIndex() + 1, Playlist::Reason::Library);
			}

		private:
			Playlist::Accessor* m_playlistAccessor;
			Playlist::Creator* m_playlistCreator;
			PlayManager* m_playManager;
	};
}

LibraryPlaylistInteractor* LibraryPlaylistInteractor::create(Playlist::Accessor* playlistAccessor,
                                                             Playlist::Creator* playlistCreator,
                                                             PlayManager* playManager)
{
	return new LibraryPlaylistInteractorImpl(playlistAccessor, playlistCreator, playManager);
}
