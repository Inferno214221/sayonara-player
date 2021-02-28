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

#include "Playlist/LibraryPlaylistInteractorImpl.h"

#include "Interfaces/PlayManager.h"

#include "Playlist/PlaylistHandler.h"
#include "Playlist/Playlist.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Playlist/PlaylistMode.h"

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
}

struct LibraryPlaylistInteractorImpl::Private
{
	Playlist::Handler* playlistHandler;
	PlayManager* playManager;

	Private(Playlist::Handler* playlistHandler, PlayManager* playManager) :
		playlistHandler {playlistHandler},
		playManager {playManager} {}
};

LibraryPlaylistInteractorImpl::LibraryPlaylistInteractorImpl(Playlist::Handler* playlistHandler,
                                                             PlayManager* playManager)
{
	m = Pimpl::make<Private>(playlistHandler, playManager);
}

LibraryPlaylistInteractorImpl::~LibraryPlaylistInteractorImpl() noexcept = default;

void LibraryPlaylistInteractorImpl::createPlaylist(const QStringList& tracks, bool createNewPlaylist)
{
	createPlaylistFromList(tracks, createNewPlaylist, m->playlistHandler);
	applyPlaylistActionAfterDoubleClick(m->playManager, m->playlistHandler);
}

void LibraryPlaylistInteractorImpl::createPlaylist(const MetaDataList& tracks, bool createNewPlaylist)
{
	createPlaylistFromList(tracks, createNewPlaylist, m->playlistHandler);
	applyPlaylistActionAfterDoubleClick(m->playManager, m->playlistHandler);
}

void LibraryPlaylistInteractorImpl::append(const MetaDataList& tracks)
{
	auto playlist = m->playlistHandler->activePlaylist();
	playlist->appendTracks(tracks);
}

void LibraryPlaylistInteractorImpl::insertAfterCurrentTrack(const MetaDataList& tracks)
{
	auto playlist = m->playlistHandler->activePlaylist();
	playlist->insertTracks(tracks, playlist->currentTrackIndex() + 1);
}