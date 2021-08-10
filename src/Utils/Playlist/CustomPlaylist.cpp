/* CustomPlaylist.cpp */

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

#include "CustomPlaylist.h"
#include "Utils/MetaData/MetaDataList.h"

struct CustomPlaylist::Private
{
	int id {-1};
	QString name;
	bool isTemporary {true};
	int tracksToFetch {0};
	MetaDataList tracks;

	Private() = default;
	~Private() = default;

	Private(const Private& other) = default;
	Private(Private&& other) noexcept = default;

	Private& operator=(const Private& other) = default;
	Private& operator=(Private&& other) noexcept = default;
};

CustomPlaylist::CustomPlaylist()
{
	m = Pimpl::make<Private>();
}

CustomPlaylist::~CustomPlaylist() = default;

CustomPlaylist::CustomPlaylist(const CustomPlaylist& other)
{
	m = Pimpl::make<Private>(*other.m);
}

CustomPlaylist::CustomPlaylist(CustomPlaylist&& other) noexcept
{
	m = Pimpl::make<Private>(std::move(*other.m));
}

CustomPlaylist& CustomPlaylist::operator=(const CustomPlaylist& other)
{
	*m = *(other.m);
	return *this;
}

CustomPlaylist& CustomPlaylist::operator=(CustomPlaylist&& other) noexcept
{
	*m = std::move(*(other.m));
	return *this;
}

int CustomPlaylist::id() const { return m->id; }

void CustomPlaylist::setId(int id) { m->id = id; }

QString CustomPlaylist::name() const { return m->name; }

void CustomPlaylist::setName(const QString& name) { m->name = name; }

bool CustomPlaylist::isTemporary() const { return m->isTemporary; }

void CustomPlaylist::setTemporary(bool temporary) { m->isTemporary = temporary; }

MetaDataList CustomPlaylist::tracks() const { return m->tracks; }

void CustomPlaylist::setTracks(const MetaDataList& tracks) { m->tracks = tracks; }

void CustomPlaylist::setTracks(MetaDataList&& tracks) { m->tracks = std::move(tracks); }

int CustomPlaylist::tracksToFetch() const { return m->tracksToFetch; }

void CustomPlaylist::setTracksToFetch(int track) { m->tracksToFetch = track; }



