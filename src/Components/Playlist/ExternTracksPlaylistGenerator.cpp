/* ExternTracksPlaylistGenerator.cpp */

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

#include "ExternTracksPlaylistGenerator.h"
#include "PlaylistHandler.h"
#include "Playlist.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language/Language.h"

#include <QStringList>

struct ExternTracksPlaylistGenerator::Private
{
	int playlistId;
	int playlistIndex;
	bool isPlayAllowed;

	Private() :
		playlistId(-1),
		playlistIndex(-1),
		isPlayAllowed(true)
	{}

	PlaylistConstPtr addNewPlaylist(const QStringList& paths)
	{
		auto* plh = Playlist::Handler::instance();

		QString name = plh->requestNewPlaylistName();

		this->playlistIndex = plh->createPlaylist(paths, name, true);
		this->playlistId = plh->playlist(this->playlistIndex)->id();

		return plh->playlist(this->playlistIndex);
	}
};

ExternTracksPlaylistGenerator::ExternTracksPlaylistGenerator()
{
	m = Pimpl::make<Private>();
}

ExternTracksPlaylistGenerator::~ExternTracksPlaylistGenerator() = default;

void ExternTracksPlaylistGenerator::addPaths(const QStringList& paths)
{
	if(paths.isEmpty()){
		m->isPlayAllowed = false;
		return;
	}

	auto* plh = Playlist::Handler::instance();

	PlaylistConstPtr pl;

	int index = -1;
	m->isPlayAllowed = true;

	if(m->playlistId < 0)
	{
		pl = m->addNewPlaylist(paths);
	}

	else
	{
		for(int i=0; i<plh->count(); i++)
		{
			auto tmp_pl = plh->playlist(i);
			if(tmp_pl->id() == m->playlistId)
			{
				index = i;
				break;
			}
		}

		if(index < 0)
		{ // the playlist was closed/deleted in the meanwhile
			pl = m->addNewPlaylist(paths);
		}

		else
		{
			pl = plh->playlist(index);

			Playlist::Mode mode = pl->mode();
			if(Playlist::Mode::isActiveAndEnabled(mode.append()))
			{ // append new tracks
				plh->appendTracks(paths, index);
				m->isPlayAllowed = false;
			}

			else
			{ // clear everything and overwrite
				plh->resetPlaylist(index);
				plh->appendTracks(paths, index);
			}
		}
	}
}

void ExternTracksPlaylistGenerator::changeTrack()
{
	auto* plh = Playlist::Handler::instance();
	PlaylistConstPtr pl = plh->playlist(m->playlistIndex);

	if(pl && pl->count() > 0)
	{
		plh->changeTrack(0, m->playlistIndex);
	}
}

bool ExternTracksPlaylistGenerator::isPlayAllowed() const
{
	return m->isPlayAllowed;
}
