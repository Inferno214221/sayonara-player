/* PlaylistStopBehavior.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "PlaylistStopBehavior.h"
#include "Playlist.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"

namespace Playlist
{
	struct StopBehavior::Private
	{
		Playlist* playlist;
		int indexBeforeStop {-1};
		Id idBeforeStop {-1};

		explicit Private(Playlist* playlist) :
			playlist {playlist} {}
	};

	StopBehavior::StopBehavior(Playlist* playlist)
	{
		m = Pimpl::make<Private>(playlist);
	}

	StopBehavior::~StopBehavior() = default;

	int StopBehavior::restoreTrackBeforeStop()
	{
		const auto& tracks = m->playlist->tracks();
		const auto it = Util::Algorithm::find(tracks, [&](const auto& track) {
			return (track.id() == m->idBeforeStop);
		});

		if(it == tracks.end())
		{
			setTrackIndexBeforeStop(-1);
			return -1;
		}

		else
		{
			m->indexBeforeStop = static_cast<int>(std::distance(tracks.begin(), it));
		}

		return m->indexBeforeStop;
	}

	int StopBehavior::trackIndexBeforeStop() const
	{
		return m->indexBeforeStop;
	}

	void StopBehavior::setTrackIndexBeforeStop(const int index)
	{
		const auto& tracks = m->playlist->tracks();
		if(Util::between(index, tracks))
		{
			m->indexBeforeStop = index;
			m->idBeforeStop = tracks[index].id();
		}
		else
		{
			m->indexBeforeStop = -1;
			m->idBeforeStop = -1;
		}

		SetSetting(Set::PL_LastTrackBeforeStop, m->indexBeforeStop);
	}
}