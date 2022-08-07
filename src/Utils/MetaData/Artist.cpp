/* Artist.cpp */

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

#include "Utils/MetaData/Artist.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"

#include <QHash>
#include <QVariant>

struct Artist::Private
{
	ArtistId id {-1};
	uint16_t songcount {0};
	HashValue artistIndex {0};
};

uint16_t Artist::songcount() const { return m->songcount; }

void Artist::setSongcount(const uint16_t& value) { m->songcount = value; }

ArtistId Artist::id() const { return m->id; }

void Artist::setId(const ArtistId& value) { m->id = value; }

Artist::Artist() :
	m {Pimpl::make<Private>()} {}

Artist::Artist(const Artist& other) :
	LibraryItem(other),
	m {Pimpl::make<Private>(*other.m)} {}

Artist::Artist(Artist&& other) noexcept :
	LibraryItem(std::move(other)),
	m {Pimpl::make<Private>(std::move(*other.m))} {}

Artist& Artist::operator=(const Artist& other)
{
	LibraryItem::operator=(other);
	*m = *(other.m);

	return *this;
}

Artist& Artist::operator=(Artist&& other) noexcept
{
	LibraryItem::operator=(std::move(other));
	*m = std::move(*other.m);

	return *this;
}

Artist::~Artist() = default;

QString Artist::name() const { return artistPool().value(m->artistIndex); }

void Artist::setName(const QString& artist)
{
	const auto hashed = qHash(artist);
	if(!artistPool().contains(hashed))
	{
		artistPool().insert(hashed, artist);
	}

	m->artistIndex = hashed;
}

QString ArtistList::majorArtist(const QStringList& artists)
{
	if(artists.isEmpty())
	{
		return "";
	}

	if(artists.count() == 1)
	{
		return artists.first().toLower().trimmed();
	}

	QHash<QString, int> map;
	for(const auto& artist: artists)
	{
		const auto lowerCase = artist.toLower().trimmed();

		// count appearance of artist
		if(!map.contains(lowerCase))
		{
			map.insert(lowerCase, 1);
		}
		else
		{
			map[lowerCase] = map.value(lowerCase) + 1;
		};
	}

	// n_appearances have to be at least 2/3 of all appearances
	const auto keys = map.keys();
	for(const auto& artist: keys)
	{
		const auto count = map.value(artist);
		if(count * 3 > artists.count() * 2)
		{
			return artist;
		}
	}

	return Lang::get(Lang::Various);
}

int ArtistList::count() const { return static_cast<int>(size()); }

ArtistList& ArtistList::operator<<(const Artist& artist)
{
	push_back(artist);
	return *this;
}

ArtistList& ArtistList::appendUnique(const ArtistList& other)
{
	for(const auto& otherArtist: other)
	{
		const auto otherId = otherArtist.id();
		const auto contains = std::any_of(begin(), end(), [otherId](const auto& artist) {
			return artist.id() == otherId;
		});

		if(!contains)
		{
			push_back(otherArtist);
		}
	}

	return *this;
}
