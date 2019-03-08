/* Album.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
	std::list<HashValue> artist_idxs;
	std::list<HashValue> album_artist_idxs;
	QStringList			 path_hint;
	HashValue album_idx;

	Private() {}
	~Private() {}

	Private(const Private& other) :
		CASSIGN(artist_idxs),
		CASSIGN(album_artist_idxs),
		CASSIGN(path_hint),
		CASSIGN(album_idx)
	{}

	Private(Private&& other) :
		CMOVE(artist_idxs),
		CMOVE(album_artist_idxs),
		CMOVE(path_hint),
		CMOVE(album_idx)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(artist_idxs);
		ASSIGN(album_artist_idxs);
		ASSIGN(path_hint);
		ASSIGN(album_idx);

		return *this;
	}

	Private& operator=(Private&& other)
	{
		MOVE(artist_idxs);
		MOVE(album_artist_idxs);
		MOVE(path_hint);
		MOVE(album_idx);

		return *this;
	}
};

Album::Album() :
	LibraryItem(),
	id(-1),
	length_sec(0),
	num_songs(0),
	year(0),
	n_discs(1),
	rating(0),
	is_sampler(false)
{
	m = Pimpl::make<Private>();
}

Album::Album(const Album& other) :
	LibraryItem(other),
	CASSIGN(discnumbers),
	CASSIGN(id),
	CASSIGN(length_sec),
	CASSIGN(num_songs),
	CASSIGN(year),
	CASSIGN(n_discs),
	CASSIGN(rating),
	CASSIGN(is_sampler)
{
	m = Pimpl::make<Private>(*(other.m));
}

Album::Album(Album&& other) :
	LibraryItem(std::move(other)),
	CMOVE(discnumbers),
	CMOVE(id),
	CMOVE(length_sec),
	CMOVE(num_songs),
	CMOVE(year),
	CMOVE(n_discs),
	CMOVE(rating),
	CMOVE(is_sampler)
{
	m = Pimpl::make<Private>(std::move(*(other.m)));
}

Album& Album::operator=(const Album& other)
{
	LibraryItem::operator =(other);

	ASSIGN(discnumbers);
	ASSIGN(id);
	ASSIGN(length_sec);
	ASSIGN(num_songs);
	ASSIGN(year);
	ASSIGN(n_discs);
	ASSIGN(rating);
	ASSIGN(is_sampler);

	*m = *(other.m);

	return *this;
}

Album& Album::operator=(Album&& other)
{
	LibraryItem::operator = (std::move(other));

	MOVE(discnumbers);
	MOVE(id);
	MOVE(length_sec);
	MOVE(num_songs);
	MOVE(year);
	MOVE(n_discs);
	MOVE(rating);
	MOVE(is_sampler);

	*m = std::move(*(other.m));

	return *this;
}


Album::~Album() {}


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
	str += QString::number(num_songs) + " Songs, " + QString::number(length_sec) + "sec";

	return str;
}


bool AlbumList::contains(AlbumId album_id) const
{
	for(auto it=this->begin(); it!=this->end(); it++){
		if(it->id == album_id){
			return true;
		}
	}

	return false;
}

int AlbumList::count() const
{
	return static_cast<int>(this->size());
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
		if(!this->contains(it->id)){
			this->push_back(*it);
		}
	}

	return *this;
}

void AlbumList::sort(::Library::SortOrder so)
{
	MetaDataSorting::sort_albums(*this, so);
}
