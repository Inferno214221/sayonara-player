/* PlaylistLoader.cpp */

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

#include "PlaylistLoader.h"
#include "PlaylistDBWrapper.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Settings/Settings.h"

using Playlist::LoaderImpl;

namespace Algorithm = Util::Algorithm;

struct LoaderImpl::Private
{
	QList<CustomPlaylist> playlists;
	int lastPlaylistId;
	int lastTrackIndex;

	Private() :
		lastPlaylistId(GetSetting(Set::PL_LastPlaylist)),
		lastTrackIndex{-1}
	{}
};

LoaderImpl::LoaderImpl() :
	Loader()
{
	m = Pimpl::make<Private>();

	auto playlistDbConnector = std::make_unique<DBWrapper>();

	const auto loadTemporaryPlaylists = GetSetting(Set::PL_LoadTemporaryPlaylists);
	const auto loadSavedPlaylists = GetSetting(Set::PL_LoadSavedPlaylists);
	const auto loadLastTrackBeforeStop = GetSetting(Set::PL_RememberTrackAfterStop);

	m->lastTrackIndex = GetSetting(Set::PL_LastTrack);
	if(m->lastTrackIndex == -1 && loadLastTrackBeforeStop)
	{
		m->lastTrackIndex = GetSetting(Set::PL_LastTrackBeforeStop);
	}

	bool success;
	if(loadSavedPlaylists && loadTemporaryPlaylists)
	{
		success = playlistDbConnector->getAllPlaylists(m->playlists);
	}

	else if(loadSavedPlaylists && !loadTemporaryPlaylists)
	{
		success = playlistDbConnector->getNonTemporaryPlaylists(m->playlists);
	}

	else if(!loadSavedPlaylists && loadTemporaryPlaylists)
	{
		success = playlistDbConnector->getTemporaryPlaylists(m->playlists);
	}

	else
	{ // no playlist loading
		success = false;
	}

	if(!success)
	{
		return;
	}

	if(m->lastPlaylistId >= 0)
	{
		const auto hasPlaylistId = Algorithm::contains(m->playlists, [&](const auto& playlist) {
			return (m->lastPlaylistId == playlist.id());
		});

		if(!hasPlaylistId)
		{
			m->playlists.prepend(playlistDbConnector->getPlaylistById(m->lastPlaylistId));
		}
	}
}

LoaderImpl::~LoaderImpl() = default;

int LoaderImpl::getLastPlaylistIndex() const
{
	return Util::Algorithm::indexOf(m->playlists, [&](const auto& playlist) {
		return (playlist.id() == m->lastPlaylistId);
	});
}

int LoaderImpl::getLastTrackIndex() const
{
	if(!GetSetting(Set::PL_LoadLastTrack))
	{
		return -1;
	}

	const auto lastPlaylistIndex = getLastPlaylistIndex();
	if(!Util::between(lastPlaylistIndex, m->playlists.size()))
	{
		return -1;
	}

	const auto trackCount = m->playlists[lastPlaylistIndex].count();
	if(!Util::between(m->lastTrackIndex, trackCount))
	{
		return -1;
	}

	return m->lastTrackIndex;
}

const QList<CustomPlaylist>& LoaderImpl::playlists() const
{
	return m->playlists;
}
