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
#include "Utils/Utils.h"

#include <QDir>
#include <QUrl>
#include <QVariant>
#include <QStringList>
#include <QHash>
#include <QGlobalStatic>
#include <QDateTime>

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
	uint64_t		createdate;
	uint64_t		modifydate;
	MilliSeconds	duration_ms;
	Filesize		filesize;
	TrackID			id;
	LibraryId		library_id;
	ArtistId		artist_id;
	AlbumId			album_id;
	ArtistId		album_artist_id;
	HashValue		album_artist_idx;
	HashValue		album_idx;
	HashValue		artist_idx;
	Bitrate			bitrate;
	TrackNum		tracknum;
	Year			year;
	Disc			discnumber;
	Disc			disc_count;
	Rating			rating;
	RadioMode       radio_mode;
	bool			is_extern;
	bool			is_disabled;

	Private() :
		createdate(0),
		modifydate(0),
		duration_ms(0),
		filesize(0),
		id(-1),
		library_id(-1),
		artist_id(-1),
		album_id(-1),
		album_artist_id(-1),
		album_artist_idx(0),
		album_idx(0),
		artist_idx(0),
		bitrate(0),
		tracknum(0),
		year(0),
		discnumber(0),
		disc_count(1),
		rating(Rating::Zero),
		radio_mode(RadioMode::Off),
		is_extern(false),
		is_disabled(false)
	{}

	Private(const Private& other) :
		CASSIGN(title),
		CASSIGN(comment),
		CASSIGN(filepath),
		CASSIGN(genres),
		CASSIGN(createdate),
		CASSIGN(modifydate),
		CASSIGN(duration_ms),
		CASSIGN(filesize),
		CASSIGN(id),
		CASSIGN(library_id),
		CASSIGN(artist_id),
		CASSIGN(album_id),
		CASSIGN(album_artist_id),
		CASSIGN(album_artist_idx),
		CASSIGN(album_idx),
		CASSIGN(artist_idx),
		CASSIGN(bitrate),
		CASSIGN(tracknum),
		CASSIGN(year),
		CASSIGN(discnumber),
		CASSIGN(disc_count),
		CASSIGN(rating),
		CASSIGN(radio_mode),
		CASSIGN(is_extern),
		CASSIGN(is_disabled)
	{
		sp_log(Log::Crazy, this) << "Copy Constr";
	}

	Private(Private&& other) noexcept :
		CMOVE(title),
		CMOVE(comment),
		CMOVE(filepath),
		CMOVE(genres),
		CMOVE(createdate),
		CMOVE(modifydate),
		CMOVE(duration_ms),
		CMOVE(filesize),
		CMOVE(id),
		CMOVE(library_id),
		CMOVE(artist_id),
		CMOVE(album_id),
		CMOVE(album_artist_id),
		CMOVE(album_artist_idx),
		CMOVE(album_idx),
		CMOVE(artist_idx),
		CMOVE(bitrate),
		CMOVE(tracknum),
		CMOVE(year),
		CMOVE(discnumber),
		CMOVE(disc_count),
		CMOVE(rating),
		CMOVE(radio_mode),
		CMOVE(is_extern),
		CMOVE(is_disabled)
	{
		sp_log(Log::Crazy, this) << "Move Constr";
	}

	Private& operator=(const Private& other)
	{
		ASSIGN(title);
		ASSIGN(comment);
		ASSIGN(filepath);
		ASSIGN(genres);
		ASSIGN(createdate);
		ASSIGN(modifydate);
		ASSIGN(duration_ms);
		ASSIGN(filesize);
		ASSIGN(id);
		ASSIGN(library_id);
		ASSIGN(artist_id);
		ASSIGN(album_id);
		ASSIGN(album_artist_id);
		ASSIGN(album_artist_idx);
		ASSIGN(album_idx);
		ASSIGN(artist_idx);
		ASSIGN(bitrate);
		ASSIGN(tracknum);
		ASSIGN(year);
		ASSIGN(discnumber);
		ASSIGN(disc_count);
		ASSIGN(rating);
		ASSIGN(radio_mode);
		ASSIGN(is_extern);
		ASSIGN(is_disabled);

		return *this;
	}

	Private& operator=(Private&& other) noexcept
	{
		MOVE(title);
		MOVE(comment);
		MOVE(filepath);
		MOVE(genres);
		MOVE(createdate);
		MOVE(modifydate);
		MOVE(duration_ms);
		MOVE(filesize);
		MOVE(id);
		MOVE(library_id);
		MOVE(artist_id);
		MOVE(album_id);
		MOVE(album_artist_id);
		MOVE(album_artist_idx);
		MOVE(album_idx);
		MOVE(artist_idx);
		MOVE(bitrate);
		MOVE(tracknum);
		MOVE(year);
		MOVE(discnumber);
		MOVE(disc_count);
		MOVE(rating);
		MOVE(radio_mode);
		MOVE(is_extern);
		MOVE(is_disabled);

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
			CMP(createdate) &&
			CMP(modifydate) &&
			CMP(duration_ms) &&
			CMP(filesize) &&
			CMP(id) &&
			CMP(library_id) &&
			CMP(artist_id) &&
			CMP(album_id) &&
			CMP(album_artist_id) &&
			CMP(album_artist_idx) &&
			CMP(album_idx) &&
			CMP(artist_idx) &&
			CMP(comment) &&
			CMP(filepath) &&
			CMP(bitrate) &&
			CMP(tracknum) &&
			CMP(year) &&
			CMP(discnumber) &&
			CMP(disc_count) &&
			CMP(rating) &&
			CMP(radio_mode) &&
			CMP(is_extern) &&
			CMP(is_disabled)
		);
	}
};

Disc MetaData::discnumber() const
{
    return m->discnumber;
}

void MetaData::set_discnumber(const Disc& d)
{
    m->discnumber = d;
}

Disc MetaData::disc_count() const
{
    return m->disc_count;
}

void MetaData::set_disc_count(const Disc& d)
{
    m->disc_count = d;
}

Bitrate MetaData::bitrate() const
{
	return m->bitrate;
}

void MetaData::set_bitrate(const Bitrate& value)
{
	m->bitrate = value;
}

TrackNum MetaData::track_number() const
{
	return m->tracknum;
}

void MetaData::set_track_number(const TrackNum& value)
{
	m->tracknum = value;
}

Year MetaData::year() const
{
	return m->year;
}

void MetaData::set_year(const Year& value)
{
	m->year = value;
}

Filesize MetaData::filesize() const
{
	return m->filesize;
}

void MetaData::set_filesize(const Filesize& value)
{
	m->filesize = value;
}

Rating MetaData::rating() const
{
	return m->rating;
}

void MetaData::set_rating(const Rating& value)
{
	m->rating = value;
}

MilliSeconds MetaData::duration_ms() const
{
	return m->duration_ms;
}

void MetaData::set_duration_ms(const MilliSeconds& value)
{
	m->duration_ms = value;
}

bool MetaData::is_extern() const
{
	return m->is_extern;
}

void MetaData::set_extern(bool value)
{
	m->is_extern = value;
}

bool MetaData::is_disabled() const
{
	return m->is_disabled;
}

void MetaData::set_disabled(bool value)
{
	m->is_disabled = value;
}

LibraryId MetaData::library_id() const
{
	return m->library_id;
}

void MetaData::set_library_id(const LibraryId& value)
{
	m->library_id = value;
}

TrackID MetaData::id() const
{
	return m->id;
}

void MetaData::set_id(const TrackID& value)
{
	m->id = value;
}

MetaData::MetaData() :
	LibraryItem()
{
	m = Pimpl::make<Private>();
#ifdef COUNT_MD
	mdc.increase();
#endif
}

MetaData::MetaData(const MetaData& other) :
	LibraryItem(other)
{
	m = Pimpl::make<Private>(*(other.m));
#ifdef COUNT_MD
	mdc.increase();
#endif
}


MetaData::MetaData(MetaData&& other) noexcept :
	LibraryItem(std::move(other))
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


MetaData& MetaData::operator=(const MetaData& other)
{
	LibraryItem::operator=(other);

	(*m) = *(other.m);

	return *this;
}

MetaData& MetaData::operator=(MetaData&& other) noexcept
{
	LibraryItem::operator=(std::move(other));

	(*m) = std::move(*(other.m));

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
	return m->is_equal(*(other.m));
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

ArtistId MetaData::artist_id() const
{
	return m->artist_id;
}

void MetaData::set_artist_id(ArtistId id)
{
	m->artist_id = id;
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

AlbumId MetaData::album_id() const
{
	return m->album_id;
}

void MetaData::set_album_id(AlbumId id)
{
	m->album_id = id;
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
		return m->artist_id;
	}

	QString str = artist_pool().value(m->album_artist_idx);
	if(str.isEmpty()){
		return m->artist_id;
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

void MetaData::set_radio_station(const QString& name)
{
	QString radio_station;
	if(name.contains("://"))
	{
		QUrl url(name);
		QString radio_station = url.host();
		if(url.port() > 0)
		{
			radio_station += QString(":%1").arg(url.port());
		}

		set_artist(radio_station);
		set_title(url.host());
	}

	else
	{
		set_artist(name);
		set_title(name);
	}

	m->album_id = -1;
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
	lst << "Rating: " << QString::number( int(rating()) );
	lst << "Disc: " << QString::number(discnumber());
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

void MetaData::set_createdate(uint64_t t)
{
	m->createdate = t;
}

uint64_t MetaData::createdate() const
{
	return m->createdate;
}

QDateTime MetaData::createdate_datetime() const
{
	return Util::int_to_date(m->createdate);
}

void MetaData::set_modifydate(uint64_t t)
{
	m->modifydate = t;
}

uint64_t MetaData::modifydate() const
{
	return m->modifydate;
}

QDateTime MetaData::modifydate_datetime() const
{
	return Util::int_to_date(m->modifydate);
}
