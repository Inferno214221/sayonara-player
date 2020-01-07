/* Album.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include <QVariant>
#include <QStringList>
#include <list>

struct Album::Private
{
	AlbumId id;
	Seconds duration_sec;
	TrackNum songcount;
	Year year;
	Rating rating;
	bool is_sampler;
	QList<Disc> discnumbers;

	std::list<HashValue> artist_idxs;
	std::list<HashValue> album_artist_idxs;
	QStringList			 path_hint;
	HashValue album_idx;

	Private() :
		id(-1),
		duration_sec(0),
		songcount(0),
		year(0),
		rating(Rating::Zero),
		is_sampler(false)
    {}

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
			CMP(duration_sec) &&
			CMP(songcount) &&
			CMP(year) &&
			CMP(rating) &&
			CMP(is_sampler) &&
			CMP(discnumbers) &&
			CMP(artist_idxs) &&
			CMP(album_artist_idxs) &&
			CMP(path_hint) &&
			CMP(album_idx)
		);
	}
};

AlbumId Album::id() const
{
    return m->id;
}

void Album::set_id(const AlbumId& id)
{
    m->id = id;
}

Seconds Album::duration_sec() const
{
    return m->duration_sec;
}

void Album::set_duration_sec(const Seconds& sec)
{
    m->duration_sec = sec;
}

TrackNum Album::songcount() const
{
    return m->songcount;
}

void Album::set_songcount(const TrackNum& songcount)
{
    m->songcount = songcount;
}

Year Album::year() const
{
    return m->year;
}

void Album::set_year(const Year& year)
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

void Album::set_rating(const Rating& rating)
{
    m->rating = rating;
}

bool Album::is_sampler() const
{
    return (m->artist_idxs.size() > 1);
}

QList<Disc> Album::discnumbers() const
{
    return m->discnumbers;
}

void Album::set_discnumbers(const QList<Disc>& discnumbers)
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

Album::Album(Album&& other) noexcept  :
	LibraryItem(std::move(other))
{
	m = Pimpl::make<Private>(std::move(*(other.m)));
}

Album& Album::operator=(const Album& other)
{
	LibraryItem::operator =(other);
	*m = *(other.m);

	return *this;
}

Album& Album::operator=(Album&& other) noexcept
{
	LibraryItem::operator = (std::move(other));
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
	return album_pool().value(m->album_idx);
}

void Album::set_name(const QString& name)
{
	HashValue hashed = qHash(name);

	if(!album_pool().contains(hashed))
	{
		album_pool().insert(hashed, name);
	}

	m->album_idx = hashed;
}

QStringList Album::artists() const
{
	QStringList lst;

	for(const HashValue& v : m->artist_idxs)
	{
		lst << artist_pool().value(v);
	}

	return lst;
}

void Album::set_artists(const QStringList& artists)
{
	m->artist_idxs.clear();

	for(const QString& artist : artists)
	{
		HashValue hashed = qHash(artist);

		if(!artist_pool().contains(hashed))
		{
			artist_pool().insert(hashed, artist);
		}

		m->artist_idxs.push_back(hashed);
	}
}

QStringList Album::album_artists() const
{
	QStringList lst;

	for(const HashValue& v : m->album_artist_idxs)
	{
		lst << artist_pool().value(v);
	}

	return lst;
}

void Album::set_album_artists(const QStringList &album_artists)
{
	m->album_artist_idxs.clear();

	for(const QString& artist : album_artists)
	{
		HashValue hashed = qHash(artist);

		if(!artist_pool().contains(hashed))
		{
			artist_pool().insert(hashed, artist);
		}

		m->album_artist_idxs.push_back(hashed);
	}
}

QStringList Album::path_hint() const
{
	return m->path_hint;
}

void Album::set_path_hint(const QStringList& paths)
{
	m->path_hint = paths;
	m->path_hint.removeDuplicates();
}

QVariant Album::toVariant(const Album& album)
{
	QVariant var;
	var.setValue(album);
	return var;
}


bool Album::fromVariant(const QVariant& v, Album& album) {
	if( !v.canConvert<Album>() ) return false;
	album =	v.value<Album>();
	return true;
}

QString Album::to_string() const
{
	QString str("Album: ");
	str += name() + " by " + artists().join(",");
	str += QString::number(m->songcount) + " Songs, " + QString::number(m->duration_sec) + "sec";

	return str;
}


bool AlbumList::contains(AlbumId album_id) const
{
	for(auto it=this->begin(); it!=this->end(); it++){
		if(it->id() == album_id){
			return true;
		}
	}

	return false;
}

int AlbumList::count() const
{
	return int(this->size());
}

AlbumList& AlbumList::operator <<(const Album &album)
{
	this->push_back(album);
	return *this;
}

Album AlbumList::first() const
{
	if(this->empty()){
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

AlbumList& AlbumList::append_unique(const AlbumList& other)
{
	for(auto it = other.begin(); it != other.end(); it++)
	{
		if(!this->contains(it->id())){
			this->push_back(*it);
		}
	}

	return *this;
}

void AlbumList::sort(::Library::SortOrder so)
{
	MetaDataSorting::sort_albums(*this, so);
}
