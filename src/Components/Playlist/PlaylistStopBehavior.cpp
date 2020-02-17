/* PlaylistStopBehavior.cpp */

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

#include "PlaylistStopBehavior.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"

namespace Algorithm=Util::Algorithm;
using Playlist::StopBehavior;

struct StopBehavior::Private
{
	int indexBeforeStop;
	Id	idBeforeStop;
};

Playlist::StopBehavior::StopBehavior()
{
	m = Pimpl::make<Private>();
}

Playlist::StopBehavior::~StopBehavior() = default;

int Playlist::StopBehavior::restoreTrackBeforeStop()
{
	const MetaDataList& v_md = tracks();
	auto it = Algorithm::find(v_md, [=](const MetaData& md){
		return (md.id() == m->idBeforeStop);
	});

	if(it == v_md.end()) {
		setTrackIndexBeforeStop(-1);
		return -1;
	}

	else {
		m->indexBeforeStop = std::distance(v_md.begin(), it);
	}

	return m->indexBeforeStop;
}

int Playlist::StopBehavior::trackIndexBeforeStop() const
{
	return m->indexBeforeStop;
}

void Playlist::StopBehavior::setTrackIndexBeforeStop(int idx)
{
	bool valid = Util::between(idx, tracks().count());
	if(valid)
	{
		m->indexBeforeStop = idx;
		m->idBeforeStop = tracks().at(idx).id();
	}

	else {
		m->indexBeforeStop = -1;
		m->idBeforeStop = -1;
	}

	SetSetting(Set::PL_LastTrackBeforeStop, m->indexBeforeStop);
}
