/* PlaylistLoader.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

	int						last_playlist_idx;
	int						last_track_idx;

	Private() :
		last_playlist_idx(-1),
		last_track_idx(-1)
	{}
};

Loader::Loader(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	auto playlist_db_connector = std::make_shared<DBWrapper>();

	bool success=false;

	bool load_playlists = (GetSetting(Set::PL_LoadSavedPlaylists) || GetSetting(Set::PL_LoadTemporaryPlaylists));
	bool load_temporary_playlists = GetSetting(Set::PL_LoadTemporaryPlaylists);
	bool load_saved_playlists = GetSetting(Set::PL_LoadSavedPlaylists);
	bool load_last_track = GetSetting(Set::PL_LoadLastTrack);
	int saved_playlist_id = GetSetting(Set::PL_LastPlaylist);
	int saved_track_idx = GetSetting(Set::PL_LastTrack);

	bool load_last_track_before_stop = GetSetting(Set::PL_RememberTrackAfterStop);
	if(saved_track_idx == -1 && load_last_track_before_stop){
		saved_track_idx = GetSetting(Set::PL_LastTrackBeforeStop);
	}

	// we don't load any playlists
	if(!load_playlists)
	{
		CustomPlaylists playlists;
		success = playlist_db_connector->get_temporary_playlists(playlists);

		if(success)
		{
			for(const CustomPlaylist& pl : Algorithm::AsConst(playlists)){
				playlist_db_connector->delete_playlist(pl.id());
			}
		}

		return;
	}

	if(load_temporary_playlists && !load_saved_playlists){
		success = playlist_db_connector->get_temporary_playlists(m->playlists);
	}

	else if(load_saved_playlists && !load_temporary_playlists){
		success = playlist_db_connector->get_non_temporary_playlists(m->playlists);
	}

	else if(load_saved_playlists && load_temporary_playlists){
		success = playlist_db_connector->get_all_playlists(m->playlists);
	}

	if(!success){
		return;
	}

	if(saved_playlist_id >= 0)
	{
		bool has_playlist_id = Algorithm::contains(m->playlists, [&saved_playlist_id](const CustomPlaylist& pl)
		{
			return (saved_playlist_id == pl.id());
		});

		if(!has_playlist_id){
			m->playlists.prepend(playlist_db_connector->get_playlist_by_id(saved_playlist_id));
		}
	}

	for(int i=0; i<m->playlists.size(); i++)
	{
		// we load all temporary playlist
		bool add_playlist = false;
		CustomPlaylist pl = m->playlists[i];

		if(pl.name().trimmed().isEmpty() ||
		   pl.size() == 0)
		{
			playlist_db_connector->delete_playlist(pl.id());
			m->playlists.removeAt(i);

			i--;
			continue;
		}


		// playlist maybe permanent or temporary
		// but this was the last one

		if(pl.id() == saved_playlist_id){
			if( Util::between(saved_track_idx, pl) )
			{
				if(load_last_track)
				{
					m->last_track_idx = saved_track_idx;
					m->last_playlist_idx = i;

					add_playlist = true;
				}
			}
		}


		if(pl.temporary())
		{
			if(load_temporary_playlists){
				add_playlist = true;
			}

			else if(!add_playlist){
				playlist_db_connector->delete_playlist(pl.id());
			}
		}

		else
		{
			if(load_saved_playlists){
				add_playlist = true;
			}
		}

		if(!add_playlist)
		{
			m->playlists.removeAt(i);
			i--;
		}
	}
}

Loader::~Loader() = default;

CustomPlaylists Loader::get_playlists() const
{
	return m->playlists;
}

int	Loader::get_last_playlist_idx() const
{
	if( !Util::between(m->last_playlist_idx, m->playlists))
	{
		return -1;
	}

	return m->last_playlist_idx;
}

int	Loader::get_last_track_idx() const
{
	int n_tracks;
	if(!Util::between(m->last_playlist_idx, m->playlists.size())){
		return -1;
	}

	n_tracks = m->playlists[m->last_playlist_idx].count();
	if(!Util::between(m->last_track_idx, n_tracks))
	{
		 return -1;
	}

	return m->last_track_idx;
}

int Loader::create_playlists()
{
	Handler* plh = Handler::instance();

	// no playlists found
	if( m->playlists.isEmpty() )
	{
		int idx = plh->create_empty_playlist();
		plh->set_current_index(idx);
	}

	else
	{
		// add playlists
		for(const CustomPlaylist& pl : Algorithm::AsConst(m->playlists))
		{
			plh->create_playlist(pl);
		}
	}

	return m->playlists.size();
}
