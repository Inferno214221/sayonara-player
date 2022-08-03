/* Album.cpp */

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

#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaDataSorting.h"
#include "Utils/Language/Language.h"

#include <QVariant>
#include <QStringList>
#include <list>

struct Album::Private
{
	AlbumId id;
	Seconds durationSec;
	TrackNum songcount;
	Year year;
	Rating rating;
	bool isSampler;
	QList<Disc> discnumbers;

	std::list<HashValue> artistIndexes;
	HashValue albumArtistIndex;
	QStringList pathHint;
	HashValue albumIdx;

	Private() :
		id(-1),
		durationSec(0),
		songcount(0),
		year(0),
		rating(Rating::Zero),
		isSampler(false) {}

	~Private() = default;

	Private(const Private& other) = default;
	Private(Private&& other) noexcept = default;
	Private& operator=(const Private& other) = default;
	Private& operator=(Private&& other) = default;

	bool operator==(const Private& other) const
	{
		return
			(
				CMP(id) &&
				CMP(durationSec) &&
				CMP(songcount) &&
				CMP(year) &&
				CMP(rating) &&
				CMP(isSampler) &&
				CMP(discnumbers) &&
				CMP(artistIndexes) &&
				CMP(albumArtistIndex) &&
				CMP(pathHint) &&
				CMP(albumIdx)
			);
	}
};

AlbumId Album::id() const
{
	return m->id;
}

void Album::setId(const AlbumId& id)
{
	m->id = id;
}

Seconds Album::durationSec() const
{
	return m->durationSec;
}

void Album::setDurationSec(const Seconds& sec)
{
	m->durationSec = sec;
}

TrackNum Album::songcount() const
{
	return m->songcount;
}

void Album::setSongcount(const TrackNum& songcount)
{
	m->songcount = songcount;
}

Year Album::year() const
{
	return m->year;
}

void Album::setYear(const Year& year)
{
	m->year = year;
}

Disc Album::disccount() const
{
	QList<Disc> discs = m->discnumbers;
	discs.erase(std::unique(discs.begin(), discs.end()));
	return Disc(discs.count());
}

Rating Album::rating() const
{
	return m->rating;
}

void Album::setRating(const Rating& rating)
{
	m->rating = rating;
}

bool Album::isSampler() const
{
	return (m->artistIndexes.size() > 1);
}

QList<Disc> Album::discnumbers() const
{
	return m->discnumbers;
}

void Album::setDiscnumbers(const QList<Disc>& discnumbers)
{
	m->discnumbers = discnumbers;
}

Album::Album() :
	LibraryItem()
{
	m = Pimpl::make<Private>();
}

Album::Album(const Album& other) :
	LibraryItem(other)
{
	m = Pimpl::make<Private>(*(other.m));
}

Album::Album(Album&& other) noexcept :
	LibraryItem(std::move(other))
{
	m = Pimpl::make<Private>(std::move(*(other.m)));
}

Album& Album::operator=(const Album& other)
{
	LibraryItem::operator=(other);
	*m = *(other.m);

	return *this;
}

Album& Album::operator=(Album&& other) noexcept
{
	LibraryItem::operator=(std::move(other));
	*m = std::move(*(other.m));

	return *this;
}

bool Album::operator==(const Album& other) const
{
	return (*m == *(other.m));
}

Album::~Album() = default;

QString Album::name() const
{
	return albumPool().value(m->albumIdx);
}

void Album::setName(const QString& name)
{
	HashValue hashed = qHash(name);

	if(!albumPool().contains(hashed))
	{
		albumPool().insert(hashed, name);
	}

	m->albumIdx = hashed;
}

QStringList Album::artists() const
{
	QStringList lst;

	for(const HashValue& v: m->artistIndexes)
	{
		lst << artistPool().value(v);
	}

	return lst;
}

void Album::setArtists(const QStringList& artists)
{
	m->artistIndexes.clear();

	for(const QString& artist: artists)
	{
		HashValue hashed = qHash(artist);

		if(!artistPool().contains(hashed))
		{
			artistPool().insert(hashed, artist);
		}

		m->artistIndexes.push_back(hashed);
	}
}

QString Album::albumArtist() const
{
	return artistPool().value(m->albumArtistIndex);
}

void Album::setAlbumArtist(const QString& albumArtist)
{
	HashValue hashed = qHash(albumArtist);

	if(!artistPool().contains(hashed))
	{
		artistPool().insert(hashed, albumArtist);
	}

	m->albumArtistIndex = hashed;
}

QStringList Album::pathHint() const
{
	return m->pathHint;
}

void Album::setPathHint(const QStringList& paths)
{
	m->pathHint = paths;
	m->pathHint.removeDuplicates();
}

QVariant Album::toVariant(const Album& album)
{
	QVariant var;
	var.setValue(album);
	return var;
}

bool Album::fromVariant(const QVariant& v, Album& album)
{
	if(!v.canConvert<Album>()) { return false; }
	album = v.value<Album>();
	return true;
}

QString Album::toString() const
{
	QString str("Album: ");
	str += name() + " by " + artists().join(",");
	str += QString::number(m->songcount) + " Songs, " + QString::number(m->durationSec) + "sec";

	return str;
}

bool AlbumList::contains(AlbumId albumId) const
{
	for(auto it = this->begin(); it != this->end(); it++)
	{
		if(it->id() == albumId)
		{
			return true;
		}
	}

	return false;
}

int AlbumList::count() const
{
	return int(this->size());
}

AlbumList& AlbumList::operator<<(const Album& album)
{
	this->push_back(album);
	return *this;
}

Album AlbumList::first() const
{
	if(this->empty())
	{
		return Album();
	}

	return this->at(0);
}

Album& AlbumList::operator[](int idx)
{
	return *(this->begin() + idx);
}

const Album& AlbumList::operator[](int idx) const
{
	return *(this->begin() + idx);
}

AlbumList& AlbumList::appendUnique(const AlbumList& other)
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

void AlbumList::sort(::Library::SortOrder so)
{
	MetaDataSorting::sortAlbums(*this, so);
}
