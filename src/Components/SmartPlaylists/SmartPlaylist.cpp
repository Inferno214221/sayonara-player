/* SmartPlaylist.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "SmartPlaylist.h"

#include "Database/SmartPlaylists.h"

struct SmartPlaylist::Private
{
	int id;
	int from;
	int to;
	std::shared_ptr<SmartPlaylists::StringConverter> stringConverter {nullptr};

	Private(const int id, const int from, const int to) :
		id {id},
		from {from},
		to {to} {}
};

SmartPlaylist::SmartPlaylist(const int id, const int from, const int to) :
	m {Pimpl::make<Private>(id, from, to)} {}

SmartPlaylist::~SmartPlaylist() = default;

SmartPlaylistDatabaseEntry SmartPlaylist::toDatabaseEntry() const
{
	return SmartPlaylistDatabaseEntry {
		id(),
		classType(),
		attributesToString()
	};
}

QString SmartPlaylist::attributesToString() const
{
	return QString("%1,%2").arg(from()).arg(to());
}

int SmartPlaylist::id() const { return m->id; }

void SmartPlaylist::setId(const int id) { m->id = id; }

int SmartPlaylist::from() const { return m->from; }

void SmartPlaylist::setFrom(const int from) { m->from = from; }

int SmartPlaylist::to() const { return m->to; }

void SmartPlaylist::setTo(const int to) { m->to = to; }

SmartPlaylists::InputFormat SmartPlaylist::inputFormat() const { return SmartPlaylists::InputFormat::Text; }

bool SmartPlaylist::canFetchTracks() const { return false; }

SmartPlaylists::StringConverterPtr SmartPlaylist::createConverter() const
{
	return std::make_shared<SmartPlaylists::StringConverter>();
}

SmartPlaylists::StringConverterPtr SmartPlaylist::stringConverter() const
{
	if(m->stringConverter != nullptr)
	{
		return m->stringConverter;
	}

	m->stringConverter = createConverter();
	return m->stringConverter;
}
