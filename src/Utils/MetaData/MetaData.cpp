/* MetaData.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "Utils/MetaData/Genre.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataSorting.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Set.h"

#include <QDir>
#include <QUrl>
#include <QVariant>
#include <QStringList>
#include <QHash>
#include <QGlobalStatic>

//#define COUNT_MD
#ifdef COUNT_MD
	struct MDCounter
	{
		int c=0;
		int m=0;
		void increase()
		{
			c++;
			m++;
			sp_log(Log::Debug, this) << "Num MD: " << c << " / " <<  m;
		}

		void decrease()
		{
			c--;
			sp_log(Log::Debug, this) << "Num MD: " << c << " / " <<  m;
		}
	};

	static MDCounter mdc;
#endif

struct MetaData::Private
{
	QString			title;
	QString			comment;
	QString         filepath;
	Util::Set<GenreID> genres;
	ArtistId		album_artist_id;
	HashValue		album_artist_idx;
	HashValue		album_idx;
	HashValue		artist_idx;

	RadioMode       radio_mode;

	Private() :
		album_artist_id(-1),
		album_artist_idx(0),
		album_idx(0),
		artist_idx(0),
		radio_mode(RadioMode::Off)
	{}

	Private(const Private& other) :
		CASSIGN(title),
		CASSIGN(comment),
		CASSIGN(filepath),
		CASSIGN(genres),
		CASSIGN(album_artist_id),
		CASSIGN(album_artist_idx),
		CASSIGN(album_idx),
		CASSIGN(artist_idx),
		CASSIGN(radio_mode)
	{}

	Private(Private&& other) :
		CMOVE(title),
		CMOVE(comment),
		CMOVE(filepath),
		CMOVE(genres),
		CMOVE(album_artist_id),
		CMOVE(album_artist_idx),
		CMOVE(album_idx),
		CMOVE(artist_idx),
		CMOVE(radio_mode)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(title);
		ASSIGN(comment);
		ASSIGN(filepath);
		ASSIGN(genres);
		ASSIGN(album_artist_id);
		ASSIGN(album_artist_idx);
		ASSIGN(album_idx);
		ASSIGN(artist_idx);
		ASSIGN(radio_mode);

		return *this;
	}

	Private& operator=(Private&& other)
	{
		MOVE(title);
		MOVE(comment);
		MOVE(filepath);
		MOVE(genres);
		MOVE(album_artist_id);
		MOVE(album_artist_idx);
		MOVE(album_idx);
		MOVE(artist_idx);
		MOVE(radio_mode);

		return *this;
	}

	~Private()
	{
		genres.clear();
	}

	bool is_equal(const Private& other) const
	{
		return(
			CMP(title) &&
			CMP(genres) &&
			CMP(album_artist_id) &&
			CMP(album_artist_idx) &&
			CMP(album_idx) &&
			CMP(artist_idx) &&
			CMP(comment) &&
			CMP(filepath) &&
			CMP(radio_mode)
		);
	}
};

MetaData::MetaData() :
	LibraryItem(),
	duration_ms(0),
	filesize(0),

	id(-1),
	artist_id(-1),
	album_id(-1),

	bitrate(0),
	track_num(0),
	year(0),

	discnumber(0),
	n_discs(0),

	library_id(-1),

	rating(Rating::Zero),
	played(false),
	is_extern(false),
	pl_playing(false),
	is_disabled(false)
{
	m = Pimpl::make<Private>();
#ifdef COUNT_MD
	mdc.increase();
#endif
}

MetaData::MetaData(const MetaData& other) :
	LibraryItem(other),
	CASSIGN(duration_ms),
	CASSIGN(filesize),
	CASSIGN(id),
	CASSIGN(artist_id),
	CASSIGN(album_id),
	CASSIGN(bitrate),
	CASSIGN(track_num),
	CASSIGN(year),
	CASSIGN(discnumber),
	CASSIGN(n_discs),
	CASSIGN(library_id),
	CASSIGN(rating),
	CASSIGN(played),
	CASSIGN(is_extern),
	CASSIGN(pl_playing),
	CASSIGN(is_disabled)
{
	m = Pimpl::make<Private>(*(other.m));
#ifdef COUNT_MD
	mdc.increase();
#endif
}


MetaData::MetaData(MetaData&& other) :
	LibraryItem(other),
	CMOVE(duration_ms),
	CMOVE(filesize),
	CMOVE(id),
	CMOVE(artist_id),
	CMOVE(album_id),
	CMOVE(bitrate),
	CMOVE(track_num),
	CMOVE(year),
	CMOVE(discnumber),
	CMOVE(n_discs),
	CMOVE(library_id),
	CMOVE(rating),
	CMOVE(played),
	CMOVE(is_extern),
	CMOVE(pl_playing),
	CMOVE(is_disabled)
{
	m = Pimpl::make<Private>(
		std::move(*(other.m))
	);
#ifdef COUNT_MD
	mdc.increase();
#endif
}

MetaData::MetaData(const QString& path) :
	MetaData()
{
#ifdef COUNT_MD
	mdc.increase();
#endif
	this->set_filepath(path);
}

MetaData::~MetaData()
{
#ifdef COUNT_MD
	mdc.decrease();
#endif

	m->genres.clear();
}

QString MetaData::title() const
{
	return m->title;
}

void MetaData::set_title(const QString &title)
{
	m->title = title;
}

QString MetaData::artist() const
{
	return artist_pool().value(m->artist_idx);
}

void MetaData::set_artist(const QString& artist)
{
	HashValue hashed = qHash(artist);
	if(!artist_pool().contains(hashed))
	{
		artist_pool().insert(hashed, artist);
	}

	m->artist_idx = hashed;
}

QString MetaData::album() const
{
	return album_pool().value(m->album_idx);
}

void MetaData::set_album(const QString& album)
{
	HashValue hashed = qHash(album);

	if(!album_pool().contains(hashed))
	{
		album_pool().insert(hashed, album);
	}

	m->album_idx = hashed;
}

const QString& MetaData::comment() const
{
	return m->comment;
}

void MetaData::set_comment(const QString& comment)
{
	m->comment = comment;
}

ArtistId MetaData::album_artist_id() const
{
	if(m->album_artist_id < 0 || m->album_artist_idx == 0){
		return artist_id;
	}

	QString str = artist_pool().value(m->album_artist_idx);
	if(str.isEmpty()){
		return artist_id;
	}

	return m->album_artist_id;
}

QString MetaData::album_artist() const
{
	QString str = artist_pool().value(m->album_artist_idx);
	if(str.isEmpty()){
		return artist();
	}

	return str;
}

void MetaData::set_album_artist(const QString& album_artist, ArtistId id)
{
	HashValue hashed = qHash(album_artist);
	if(!artist_pool().contains(hashed))
	{
		artist_pool().insert(hashed, album_artist);
	}

	m->album_artist_idx = hashed;
	m->album_artist_id = id;
}

void MetaData::set_album_artist_id(ArtistId id)
{
	m->album_artist_id = id;
}


void MetaData::set_radio_station(const QString& filepath)
{
	QUrl url(filepath);
	QString radio_station = url.host();
	if(url.port() > 0)
	{
		radio_station += QString(":%1").arg(url.port());
	}

	set_artist(radio_station);
	//set_album(radio_station);
	set_title(url.host());

	album_id = -1;
}

QString MetaData::radio_station() const
{
	return artist();
}

bool MetaData::has_album_artist() const
{
	return (m->album_artist_idx > 0);
}

QString MetaData::to_string() const
{
	QStringList lst;
	lst << m->title;
	lst << "by " << this->artist() << " (" << album_artist() << ")";
	lst << "on " << this->album();
	lst << "Rating: " << QString::number( int(rating) );
	lst << "Disc: " << QString::number(discnumber);
	lst << "Filepath: " << filepath();

	return lst.join(" - ");
}


QVariant MetaData::toVariant(const MetaData& md)
{
	QVariant v;

	v.setValue<MetaData>(md);

	return v;
}

bool MetaData::fromVariant(const QVariant& v, MetaData& md)
{
	if(!v.canConvert<MetaData>() ) {
		return false;
	}

	md = v.value<MetaData>() ;
	return true;
}

QHash<GenreID, Genre>& MetaData::genre_pool() const
{
	static QHash<GenreID, Genre> pool;
	return pool;
}


MetaData& MetaData::operator=(const MetaData& other)
{
	LibraryItem::operator=(other);

	(*m) = *(other.m);

	ASSIGN(duration_ms);
	ASSIGN(filesize);
	ASSIGN(id);
	ASSIGN(artist_id);
	ASSIGN(album_id);
	ASSIGN(bitrate);
	ASSIGN(track_num);
	ASSIGN(year);
	ASSIGN(discnumber);
	ASSIGN(n_discs);
	ASSIGN(library_id);
	ASSIGN(rating);
	ASSIGN(played);
	ASSIGN(is_extern);
	ASSIGN(pl_playing);
	ASSIGN(is_disabled);

	return *this;
}

MetaData& MetaData::operator=(MetaData&& other)
{
	LibraryItem::operator=(std::move(other));

	(*m) = std::move(*(other.m));

	MOVE(duration_ms);
	MOVE(filesize);
	MOVE(id);
	MOVE(artist_id);
	MOVE(album_id);
	MOVE(bitrate);
	MOVE(track_num);
	MOVE(year);
	MOVE(discnumber);
	MOVE(n_discs);
	MOVE(library_id);
	MOVE(rating);
	MOVE(played);
	MOVE(is_extern);
	MOVE(pl_playing);
	MOVE(is_disabled);

	return *this;
}

bool MetaData::operator==(const MetaData& md) const
{
	return this->is_equal(md);
}


bool MetaData::operator!=(const MetaData& md) const
{
	return !(this->is_equal(md));
}


bool MetaData::is_equal(const MetaData& md) const
{
	QDir first_path(m->filepath);
	QDir other_path(md.filepath());

	QString s_first_path = first_path.absolutePath();
	QString s_other_path = other_path.absolutePath();

#ifdef Q_OS_UNIX
	return (s_first_path.compare(s_other_path) == 0);
#else
	return (s_first_path.compare(s_other_path, Qt::CaseInsensitive) == 0);
#endif

}

bool MetaData::is_equal_deep(const MetaData& other) const
{
	return
	(
		CMP(duration_ms) &&
		CMP(filesize) &&
		CMP(id) &&
		CMP(artist_id) &&
		CMP(album_id) &&
		CMP(bitrate) &&
		CMP(track_num) &&
		CMP(year) &&
		CMP(played) &&
		CMP(is_extern) &&
		CMP(pl_playing) &&
		CMP(is_disabled) &&
		CMP(rating) &&
		CMP(discnumber) &&
		CMP(n_discs) &&
		CMP(library_id) &&
		m->is_equal(*(other.m))
	);
}

const Util::Set<GenreID>& MetaData::genre_ids() const
{
	return m->genres;
}


Util::Set<Genre> MetaData::genres() const
{
	Util::Set<Genre> genres;

	for(GenreID genre_id : m->genres){
		genres.insert( genre_pool().value(genre_id) );
	}

	return genres;
}

void MetaData::set_genres(const Util::Set<Genre>& genres)
{
	m->genres.clear();
	for(const Genre& genre : genres)
	{
		GenreID id = genre.id();
		if(!genre_pool().contains(id))
		{
			genre_pool().insert(id, genre);
		}

		m->genres << id;
	}
}


bool MetaData::has_genre(const Genre& genre) const
{
	for(const GenreID& id : m->genres)
	{
		if(id == genre.id()){
			return true;
		}
	}

	return false;
}

bool MetaData::remove_genre(const Genre& genre)
{
	m->genres.remove(genre.id());
	return true;
}

bool MetaData::add_genre(const Genre& genre)
{
	GenreID id = genre.id();
	if(!genre_pool().contains(id))
	{
		genre_pool().insert(id, genre);
	}

	m->genres << id;

	return true;
}

void MetaData::set_genres(const QStringList& new_genres)
{
	m->genres.clear();
	for(const QString& g : new_genres)
	{
		Genre genre(g);
		add_genre(genre);
	}
}


QString MetaData::genres_to_string() const
{
	return genres_to_list().join(",");
}

QStringList MetaData::genres_to_list() const
{
	QStringList new_genres;
	for(const GenreID& id : m->genres)
	{
		new_genres << genre_pool().value(id).name();
	}

	return new_genres;
}

QString MetaData::filepath() const
{
	return m->filepath;
}


QString MetaData::set_filepath(QString filepath)
{
	bool is_local_path = false;

#ifdef Q_OS_UNIX
	if(filepath.startsWith("/")){
		is_local_path = true;
	}
#else
	if(filepath.contains(":\\") || filepath.contains("\\\\")){
		is_local_path = true;
	}
#endif

	if(is_local_path){
		QDir dir(filepath);
		m->filepath = dir.absolutePath();
		m->radio_mode = RadioMode::Off;
	}

	else if(filepath.contains("soundcloud.com")){
		m->filepath = filepath;
		m->radio_mode = RadioMode::Soundcloud;
	}

	else{
		m->filepath = filepath;
		m->radio_mode = RadioMode::Station;
	}

	return m->filepath;
}


RadioMode MetaData::radio_mode() const
{
	return m->radio_mode;
}

bool MetaData::is_valid() const
{
	return (!filepath().isEmpty());
}
