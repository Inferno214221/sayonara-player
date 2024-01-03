/* Album.cpp */

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

#include "Utils/MetaData/Album.h"
#include "Utils/Language/Language.h"

#include <QVariant>
#include <QStringList>

#include <algorithm>
#include <list>
#include <unordered_set>

struct Album::Private
{
	AlbumId id {-1};
	Seconds durationSec {0};
	TrackNum songcount {0};
	Year year {0};
	Rating rating {Rating::Zero};
	QList<Disc> discnumbers;

	std::list<HashValue> artistIndexes;
	HashValue albumArtistIndex {0};
	QStringList pathHint;
	HashValue albumIdx {0};
};

AlbumId Album::id() const { return m->id; }

void Album::setId(const AlbumId& id) { m->id = id; }

Seconds Album::durationSec() const { return m->durationSec; }

void Album::setDurationSec(const Seconds& sec) { m->durationSec = sec; }

TrackNum Album::songcount() const { return m->songcount; }

void Album::setSongcount(const TrackNum& songcount) { m->songcount = songcount; }

Year Album::year() const { return m->year; }

void Album::setYear(const Year& year) { m->year = year; }

Disc Album::disccount() const
{
	auto discs = std::unordered_set<Disc> {};
	for(const auto& disc: m->discnumbers)
	{
		discs.insert(disc);
	}

	return static_cast<Disc>(discs.size());
}

Rating Album::rating() const { return m->rating; }

void Album::setRating(const Rating& rating) { m->rating = rating; }

bool Album::isSampler() const { return (m->artistIndexes.size() > 1); }

QList<Disc> Album::discnumbers() const { return m->discnumbers; }

void Album::setDiscnumbers(const QList<Disc>& discnumbers) { m->discnumbers = discnumbers; }

Album::Album() :
	m {Pimpl::make<Private>()} {}

Album::Album(const Album& other) :
	LibraryItem(other),
	m {Pimpl::make<Private>(*(other.m))} {}

Album::Album(Album&& other) noexcept :
	LibraryItem(std::move(other)),
	m {Pimpl::make<Private>(std::move(*other.m))} {}

Album& Album::operator=(const Album& other)
{
	LibraryItem::operator=(other);
	*m = *(other.m);

	return *this;
}

Album& Album::operator=(Album&& other) noexcept
{
	LibraryItem::operator=(std::move(other));
	*m = std::move(*other.m); // NOLINT(bugprone-use-after-move)

	return *this;
}

Album::~Album() = default;

QString Album::name() const { return albumPool().value(m->albumIdx); }

void Album::setName(const QString& name)
{
	const auto hashed = qHash(name);
	if(!albumPool().contains(hashed))
	{
		albumPool().insert(hashed, name);
	}

	m->albumIdx = hashed;
}

QStringList Album::artists() const
{
	QStringList lst;

	for(const auto& artistIndex: m->artistIndexes)
	{
		lst << artistPool().value(artistIndex);
	}

	return lst;
}

void Album::setArtists(const QStringList& artists)
{
	m->artistIndexes.clear();

	for(const auto& artist: artists)
	{
		const auto hashed = qHash(artist);
		if(!artistPool().contains(hashed))
		{
			artistPool().insert(hashed, artist);
		}

		m->artistIndexes.push_back(hashed);
	}
}

QString Album::albumArtist() const { return artistPool().value(m->albumArtistIndex); }

void Album::setAlbumArtist(const QString& albumArtist)
{
	const auto hashed = qHash(albumArtist);
	if(!artistPool().contains(hashed))
	{
		artistPool().insert(hashed, albumArtist);
	}

	m->albumArtistIndex = hashed;
}

QStringList Album::pathHint() const { return m->pathHint; }

void Album::setPathHint(const QStringList& paths)
{
	m->pathHint = paths;
	m->pathHint.removeDuplicates();
}

int AlbumList::count() const { return static_cast<int>(size()); }

AlbumList& AlbumList::operator<<(const Album& album)
{
	push_back(album);
	return *this;
}

Album& AlbumList::operator[](const int idx) { return *(this->begin() + idx); }

const Album& AlbumList::operator[](const int idx) const { return *(this->begin() + idx); }

AlbumList& AlbumList::appendUnique(const AlbumList& other)
{
	for(const auto& otherAlbum: other)
	{
		const auto otherId = otherAlbum.id();
		const auto contains = std::any_of(begin(), end(), [otherId](const auto& album) {
			return album.id() == otherId;
		});

		if(!contains)
		{
			push_back(otherAlbum);
		}
	}

	return *this;
}
