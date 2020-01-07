/* ExternTracksPlaylistGenerator.cpp */

/* Copyright (C) 2011-2020 Lucio Carreras
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

#include "ExternTracksPlaylistGenerator.h"
#include "PlaylistHandler.h"
#include "Playlist.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language/Language.h"

#include <QStringList>

struct ExternTracksPlaylistGenerator::Private
{
	int playlist_id;
	int playlist_index;
	bool is_play_allowed;

	Private() :
		playlist_id(-1),
		playlist_index(-1),
		is_play_allowed(true)
	{}

	PlaylistConstPtr add_new_playlist(const QStringList& paths)
	{
		auto* plh = Playlist::Handler::instance();

		QString name = plh->request_new_playlist_name();

		this->playlist_index = plh->create_playlist(paths, name, true);
		this->playlist_id = plh->playlist(this->playlist_index)->get_id();

		return plh->playlist(this->playlist_index);
	}
};

ExternTracksPlaylistGenerator::ExternTracksPlaylistGenerator()
{
	m = Pimpl::make<Private>();
}

ExternTracksPlaylistGenerator::~ExternTracksPlaylistGenerator() = default;

void ExternTracksPlaylistGenerator::add_paths(const QStringList& paths)
{
	if(paths.isEmpty()){
		m->is_play_allowed = false;
		return;
	}

	auto* plh = Playlist::Handler::instance();

	PlaylistConstPtr pl;

	int index = -1;
	m->is_play_allowed = true;

	if(m->playlist_id < 0)
	{
		pl = m->add_new_playlist(paths);
	}

	else
	{
		for(int i=0; i<plh->count(); i++)
		{
			auto tmp_pl = plh->playlist(i);
			if(tmp_pl->get_id() == m->playlist_id)
			{
				index = i;
				break;
			}
		}

		if(index < 0)
		{ // the playlist was closed/deleted in the meanwhile
			pl = m->add_new_playlist(paths);
		}

		else
		{
			pl = plh->playlist(index);

			Playlist::Mode mode = pl->mode();
			if(Playlist::Mode::isActiveAndEnabled(mode.append()))
			{ // append new tracks
				plh->append_tracks(paths, index);
				m->is_play_allowed = false;
			}

			else
			{ // clear everything and overwrite
				plh->reset_playlist(index);
				plh->append_tracks(paths, index);
			}
		}
	}
}

void ExternTracksPlaylistGenerator::change_track()
{
	auto* plh = Playlist::Handler::instance();
	PlaylistConstPtr pl = plh->playlist(m->playlist_index);

	if(pl && pl->count() > 0)
	{
		plh->change_track(0, m->playlist_index);
	}
}

bool ExternTracksPlaylistGenerator::is_play_allowed() const
{
	return m->is_play_allowed;
}
