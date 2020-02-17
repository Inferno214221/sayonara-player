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
#include "PlaylistHandler.h"
#include "PlaylistDBWrapper.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Settings/Settings.h"

using Playlist::Loader;
using Playlist::Handler;

namespace Algorithm=Util::Algorithm;

struct Loader::Private
{
	CustomPlaylists			playlists;

	int						lastPlaylistIndex;
	int						lastTrackIndex;

	Private() :
		lastPlaylistIndex(-1),
		lastTrackIndex(-1)
	{}
};

Loader::Loader(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	auto playlistDbConnector = std::make_shared<DBWrapper>();

	bool success=false;

	bool loadPlaylists = (GetSetting(Set::PL_LoadSavedPlaylists) || GetSetting(Set::PL_LoadTemporaryPlaylists));
	bool loadTemporaryPlaylists = GetSetting(Set::PL_LoadTemporaryPlaylists);
	bool loadSavedPlaylists = GetSetting(Set::PL_LoadSavedPlaylists);
	bool loadLastTrack = GetSetting(Set::PL_LoadLastTrack);
	int savedPlaylistId = GetSetting(Set::PL_LastPlaylist);
	int savedTrackIndex = GetSetting(Set::PL_LastTrack);

	bool load_last_track_before_stop = GetSetting(Set::PL_RememberTrackAfterStop);
	if(savedTrackIndex == -1 && load_last_track_before_stop){
		savedTrackIndex = GetSetting(Set::PL_LastTrackBeforeStop);
	}

	// we don't load any playlists
	if(!loadPlaylists)
	{
		CustomPlaylists playlists;
		success = playlistDbConnector->getTemporaryPlaylists(playlists);

		if(success)
		{
			for(const CustomPlaylist& pl : Algorithm::AsConst(playlists)){
				playlistDbConnector->deletePlaylist(pl.id());
			}
		}

		return;
	}

	if(loadTemporaryPlaylists && !loadSavedPlaylists){
		success = playlistDbConnector->getTemporaryPlaylists(m->playlists);
	}

	else if(loadSavedPlaylists && !loadTemporaryPlaylists){
		success = playlistDbConnector->getNonTemporaryPlaylists(m->playlists);
	}

	else if(loadSavedPlaylists && loadTemporaryPlaylists){
		success = playlistDbConnector->getAllPlaylists(m->playlists);
	}

	if(!success){
		return;
	}

	if(savedPlaylistId >= 0)
	{
		bool hasPlaylist_id = Algorithm::contains(m->playlists, [&savedPlaylistId](const CustomPlaylist& pl)
		{
			return (savedPlaylistId == pl.id());
		});

		if(!hasPlaylist_id){
			m->playlists.prepend(playlistDbConnector->getPlaylistById(savedPlaylistId));
		}
	}

	for(int i=0; i<m->playlists.size(); i++)
	{
		// we load all temporary playlist
		bool addPlaylist = false;
		CustomPlaylist pl = m->playlists[i];

		if(pl.name().trimmed().isEmpty() ||
		   pl.size() == 0)
		{
			playlistDbConnector->deletePlaylist(pl.id());
			m->playlists.removeAt(i);

			i--;
			continue;
		}


		// playlist maybe permanent or temporary
		// but this was the last one

		if(pl.id() == savedPlaylistId){
			if( Util::between(savedTrackIndex, pl) )
			{
				if(loadLastTrack)
				{
					m->lastTrackIndex = savedTrackIndex;
					m->lastPlaylistIndex = i;

					addPlaylist = true;
				}
			}
		}


		if(pl.temporary())
		{
			if(loadTemporaryPlaylists){
				addPlaylist = true;
			}

			else if(!addPlaylist){
				playlistDbConnector->deletePlaylist(pl.id());
			}
		}

		else
		{
			if(loadSavedPlaylists){
				addPlaylist = true;
			}
		}

		if(!addPlaylist)
		{
			m->playlists.removeAt(i);
			i--;
		}
	}
}

Loader::~Loader() = default;

CustomPlaylists Loader::getPlaylists() const
{
	return m->playlists;
}

int	Loader::getLastPlaylistIndex() const
{
	if( !Util::between(m->lastPlaylistIndex, m->playlists))
	{
		return -1;
	}

	return m->lastPlaylistIndex;
}

int	Loader::getLastTrackIndex() const
{
	int trackCount;
	if(!Util::between(m->lastPlaylistIndex, m->playlists.size())){
		return -1;
	}

	trackCount = m->playlists[m->lastPlaylistIndex].count();
	if(!Util::between(m->lastTrackIndex, trackCount))
	{
		 return -1;
	}

	return m->lastTrackIndex;
}

int Loader::createPlaylists()
{
	Handler* plh = Handler::instance();

	// no playlists found
	if( m->playlists.isEmpty() )
	{
		int idx = plh->createEmptyPlaylist();
		plh->set_current_index(idx);
	}

	else
	{
		// add playlists
		for(const CustomPlaylist& pl : Algorithm::AsConst(m->playlists))
		{
			plh->createPlaylist(pl);
		}
	}

	return m->playlists.size();
}
