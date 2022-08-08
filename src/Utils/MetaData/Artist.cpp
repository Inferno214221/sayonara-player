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
	HashValue artistIndex;
	ArtistId id;
	uint16_t albumcount;
	uint16_t songcount;

	Private() :
		id(-1),
		albumcount(0),
		songcount(0) {}

	~Private() = default;

	Private(const Private& other) :
		CASSIGN(artistIndex),
		CASSIGN(id),
		CASSIGN(albumcount),
		CASSIGN(songcount) {}

	Private(Private&& other) noexcept :
		CMOVE(artistIndex),
		CMOVE(id),
		CMOVE(albumcount),
		CMOVE(songcount) {}

	Private& operator=(const Private& other)
	{
		ASSIGN(artistIndex);
		ASSIGN(id);
		ASSIGN(albumcount);
		ASSIGN(songcount);

		return *this;
	}

	Private& operator=(Private&& other) noexcept
	{
		MOVE(artistIndex);
		MOVE(id);
		MOVE(albumcount);
		MOVE(songcount);

		return *this;
	}
};

uint16_t Artist::albumcount() const
{
	return m->albumcount;
}

void Artist::setAlbumcount(const uint16_t& value)
{
	m->albumcount = value;
}

uint16_t Artist::songcount() const
{
	return m->songcount;
}

void Artist::setSongcount(const uint16_t& value)
{
	m->songcount = value;
}

ArtistId Artist::id() const
{
	return m->id;
}

void Artist::setId(const ArtistId& value)
{
	m->id = value;
}

Artist::Artist() :
	LibraryItem()
{
	m = Pimpl::make<Private>();
}

Artist::Artist(const Artist& other) :
	LibraryItem(other)
{
	m = Pimpl::make<Private>(*(other.m));
}

Artist::Artist(Artist&& other) noexcept :
	LibraryItem(std::move(other))
{
	m = Pimpl::make<Private>(std::move(*(other.m)));
}

Artist& Artist::operator=(const Artist& other)
{
	LibraryItem::operator=(other);

	*m = *(other.m);

	return *this;
}

Artist& Artist::operator=(Artist&& other) noexcept
{
	LibraryItem::operator=(std::move(other));

	*m = std::move(*(other.m));

	return *this;
}

Artist::~Artist() = default;

QString Artist::name() const
{
	return artistPool().value(m->artistIndex);
}

void Artist::setName(const QString& artist)
{
	HashValue hashed = qHash(artist);
	if(!artistPool().contains(hashed))
	{
		artistPool().insert(hashed, artist);
	}

	m->artistIndex = hashed;
}

QVariant Artist::toVariant(const Artist& artist)
{
	QVariant var;
	var.setValue(artist);
	return var;
}

bool Artist::fromVariant(const QVariant& v, Artist& artist)
{
	if(!v.canConvert<Artist>()) { return false; }

	artist = v.value<Artist>();
	return true;
}

void Artist::print() const
{
	spLog(Log::Info, this) << id() << ": " << name() << ": " << songcount() << " Songs, " << albumcount() << " Albums";
}

ArtistList::ArtistList() :
	ArtistList::Parent() {}

ArtistList::~ArtistList() {}

QString ArtistList::majorArtist(const QStringList& artists)
{
	QHash<QString, int> map;
	int n_artists = artists.size();

	if(artists.isEmpty())
	{
		return "";
	}

	if(n_artists == 1)
	{
		return artists.first().toLower().trimmed();
	}

	for(const QString& artist: artists)
	{
		QString alower = artist.toLower().trimmed();

		// count appearance of artist
		if(!map.contains(alower))
		{
			map.insert(alower, 1);
		}
		else
		{
			map[alower] = map.value(alower) + 1;
		};
	}

	// n_appearances have to be at least 2/3 of all appearances
	const QList<QString> keys = map.keys();
	for(const QString& artist: keys)
	{
		int n_appearances = map.value(artist);
		if(n_appearances * 3 > n_artists * 2)
		{
			return artist;
		}
	}

	return Lang::get(Lang::Various);
}

QString ArtistList::majorArtist() const
{
	QStringList lst;

	for(auto it = this->begin(); it != this->end(); it++)
	{
		lst << it->name();
	}

	return majorArtist(lst);
}

bool ArtistList::contains(ArtistId artistId) const
{
	for(auto it = this->begin(); it != this->end(); it++)
	{
		if(it->id() == artistId)
		{
			return true;
		}
	}

	return false;
}

int ArtistList::count() const
{
	return int(this->size());
}

ArtistList& ArtistList::operator<<(const Artist& artist)
{
	this->push_back(artist);
	return *this;
}

Artist ArtistList::first() const
{
	if(this->empty())
	{
		return Artist();
	}

	return this->at(0);
}

ArtistList& ArtistList::appendUnique(const ArtistList& other)
{
	for(auto it = other.begin(); it != other.end(); it++)
	{
		if(!this->contains(it->id()))
		{
			this->push_back(*it);
		}
	}

	return *this;
}
