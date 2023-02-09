/* LibraryPlaylistInteractorImpl.cpp */
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

#include "LibraryPlaylistInteractor.h"

#include "PlayManager/PlayManager.h"
#include "Playlist.h"
#include "PlaylistHandler.h"
#include "PlaylistModifiers.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"

namespace
{
	template<typename T>
	void createPlaylistFromList(const T& tracks, bool createNewPlaylist, PlaylistCreator* playlistCreator)
	{
		const auto name = (createNewPlaylist)
		                  ? playlistCreator->requestNewPlaylistName()
		                  : QString();

		playlistCreator->createPlaylist(tracks, name);
	}

	void applyPlaylistActionAfterDoubleClick(PlayManager* playManager, PlaylistAccessor* playlistAccessor)
	{
		if(GetSetting(Set::Lib_DC_DoNothing))
		{
			return;
		}

		const auto currentIndex = playlistAccessor->currentIndex();
		const auto currentPlaylist = playlistAccessor->playlist(currentIndex);

		if(GetSetting(Set::Lib_DC_PlayIfStopped))
		{
			if(currentPlaylist && (playManager->playstate() != PlayState::Playing))
			{
				currentPlaylist->changeTrack(0);
			}
		}

		else if(GetSetting(Set::Lib_DC_PlayImmediately))
		{
			const auto plm = GetSetting(Set::PL_Mode);
			if(currentPlaylist && (plm.append() == ::Playlist::Mode::State::Off))
			{
				currentPlaylist->changeTrack(0);
			}
		}
	}

	class LibraryPlaylistInteractorImpl :
		public LibraryPlaylistInteractor
	{
		public:

			LibraryPlaylistInteractorImpl(Playlist::Handler* playlistHandler,
			                              PlayManager* playManager) :
				m_playlistHandler {playlistHandler},
				m_playManager {playManager} {}

			~LibraryPlaylistInteractorImpl() noexcept override = default;

			void createPlaylist(const QStringList& tracks, bool createNewPlaylist) override
			{
				createPlaylistFromList(tracks, createNewPlaylist, m_playlistHandler);
				applyPlaylistActionAfterDoubleClick(m_playManager, m_playlistHandler);
			}

			void createPlaylist(const MetaDataList& tracks, bool createNewPlaylist) override
			{
				createPlaylistFromList(tracks, createNewPlaylist, m_playlistHandler);
				applyPlaylistActionAfterDoubleClick(m_playManager, m_playlistHandler);
			}

			void append(const MetaDataList& tracks) override
			{
				Playlist::appendTracks(*m_playlistHandler->activePlaylist(), tracks);
			}

			void insertAfterCurrentTrack(const MetaDataList& tracks) override
			{
				auto playlist = m_playlistHandler->activePlaylist();
				Playlist::insertTracks(*playlist, tracks, playlist->currentTrackIndex() + 1);
			}

		private:
			Playlist::Handler* m_playlistHandler;
			PlayManager* m_playManager;
	};
}

LibraryPlaylistInteractor*
LibraryPlaylistInteractor::create(Playlist::Handler* playlistHandler, PlayManager* playManager)
{
	return new LibraryPlaylistInteractorImpl(playlistHandler, playManager);
}
