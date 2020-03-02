/* MetaData.cpp */

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
	MilliSeconds	durationMs;
	Filesize		filesize;
	TrackID			id;
	LibraryId		libraryId;
	ArtistId		artistId;
	AlbumId			albumId;
	ArtistId		albumArtistId;
	HashValue		albumArtistIdx;
	HashValue		albumIdx;
	HashValue		artistIdx;
	Bitrate			bitrate;
	TrackNum		tracknum;
	Year			year;
	Disc			discnumber;
	Disc			discCount;
	Rating			rating;
	RadioMode       radioMode;
	bool			isExtern;
	bool			isDisabled;

	Private() :
		createdate(0),
		modifydate(0),
		durationMs(0),
		filesize(0),
		id(-1),
		libraryId(-1),
		artistId(-1),
		albumId(-1),
		albumArtistId(-1),
		albumArtistIdx(0),
		albumIdx(0),
		artistIdx(0),
		bitrate(0),
		tracknum(0),
		year(0),
		discnumber(0),
		discCount(1),
		rating(Rating::Zero),
		radioMode(RadioMode::Off),
		isExtern(false),
		isDisabled(false)
	{}

	Private(const Private& other) = default;
	Private(Private&& other) noexcept = default;
	Private& operator=(const Private& other) = default;
	Private& operator=(Private&& other) noexcept = default;

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
			CMP(durationMs) &&
			CMP(filesize) &&
			CMP(id) &&
			CMP(libraryId) &&
			CMP(artistId) &&
			CMP(albumId) &&
			CMP(albumArtistId) &&
			CMP(albumArtistIdx) &&
			CMP(albumIdx) &&
			CMP(artistIdx) &&
			CMP(comment) &&
			CMP(filepath) &&
			CMP(bitrate) &&
			CMP(tracknum) &&
			CMP(year) &&
			CMP(discnumber) &&
			CMP(discCount) &&
			CMP(rating) &&
			CMP(radioMode) &&
			CMP(isExtern) &&
			CMP(isDisabled)
		);
	}
};

Disc MetaData::discnumber() const
{
    return m->discnumber;
}

void MetaData::setDiscnumber(const Disc& d)
{
    m->discnumber = d;
}

Disc MetaData::discCount() const
{
    return m->discCount;
}

void MetaData::setDiscCount(const Disc& d)
{
    m->discCount = d;
}

Bitrate MetaData::bitrate() const
{
	return m->bitrate;
}

void MetaData::setBitrate(const Bitrate& value)
{
	m->bitrate = value;
}

TrackNum MetaData::trackNumber() const
{
	return m->tracknum;
}

void MetaData::setTrackNumber(const TrackNum& value)
{
	m->tracknum = value;
}

Year MetaData::year() const
{
	return m->year;
}

void MetaData::setYear(const Year& value)
{
	m->year = value;
}

Filesize MetaData::filesize() const
{
	return m->filesize;
}

void MetaData::setFilesize(const Filesize& value)
{
	m->filesize = value;
}

Rating MetaData::rating() const
{
	return m->rating;
}

void MetaData::setRating(const Rating& value)
{
	m->rating = value;
}

MilliSeconds MetaData::durationMs() const
{
	return m->durationMs;
}

void MetaData::setDurationMs(const MilliSeconds& value)
{
	m->durationMs = value;
}

bool MetaData::isExtern() const
{
	return m->isExtern;
}

void MetaData::setExtern(bool value)
{
	m->isExtern = value;
}

bool MetaData::isDisabled() const
{
	return m->isDisabled;
}

void MetaData::setDisabled(bool value)
{
	m->isDisabled = value;
}

LibraryId MetaData::libraryId() const
{
	return m->libraryId;
}

void MetaData::setLibraryid(const LibraryId& value)
{
	m->libraryId = value;
}

TrackID MetaData::id() const
{
	return m->id;
}

void MetaData::setId(const TrackID& value)
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
	this->setFilepath(path);
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
	return this->isEqual(md);
}

bool MetaData::operator!=(const MetaData& md) const
{
	return !(this->isEqual(md));
}

bool MetaData::isEqual(const MetaData& md) const
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

bool MetaData::isEqualDeep(const MetaData& other) const
{
	return m->is_equal(*(other.m));
}

QString MetaData::title() const
{
	return m->title;
}

void MetaData::setTitle(const QString& title)
{
	m->title = title.trimmed();
}

QString MetaData::artist() const
{
	return artistPool().value(m->artistIdx);
}

void MetaData::setArtist(const QString& artist)
{
	HashValue hashed = qHash(artist);
	if(!artistPool().contains(hashed))
	{
		artistPool().insert(hashed, artist);
	}

	m->artistIdx = hashed;
}

ArtistId MetaData::artistId() const
{
	return m->artistId;
}

void MetaData::setArtistId(ArtistId id)
{
	m->artistId = id;
}

QString MetaData::album() const
{
	return albumPool().value(m->albumIdx);
}

void MetaData::setAlbum(const QString& album)
{
	HashValue hashed = qHash(album);

	if(!albumPool().contains(hashed))
	{
		albumPool().insert(hashed, album);
	}

	m->albumIdx = hashed;
}

AlbumId MetaData::albumId() const
{
	return m->albumId;
}

void MetaData::setAlbumId(AlbumId id)
{
	m->albumId = id;
}

const QString& MetaData::comment() const
{
	return m->comment;
}

void MetaData::setComment(const QString& comment)
{
	m->comment = comment;
}

ArtistId MetaData::albumArtistId() const
{
	if(m->albumArtistId < 0 || m->albumArtistIdx == 0){
		return m->artistId;
	}

	QString str = artistPool().value(m->albumArtistIdx);
	if(str.isEmpty()){
		return m->artistId;
	}

	return m->albumArtistId;
}

QString MetaData::albumArtist() const
{
	QString str = artistPool().value(m->albumArtistIdx);
	if(str.isEmpty()){
		return artist();
	}

	return str;
}

void MetaData::setAlbumArtist(const QString& album_artist, ArtistId id)
{
	HashValue hashed = qHash(album_artist);
	if(!artistPool().contains(hashed))
	{
		artistPool().insert(hashed, album_artist);
	}

	m->albumArtistIdx = hashed;
	m->albumArtistId = id;
}

void MetaData::setAlbumArtistId(ArtistId id)
{
	m->albumArtistId = id;
}

void MetaData::setRadioStation(const QString& stationUrl, const QString& name)
{
	if(stationUrl.contains("://"))
	{
		QUrl url(stationUrl);
		QString host = url.host();

		if(url.port() > 0) {
			host += QString(":%1").arg(url.port());
		}

		if(name.isEmpty()) {
			setTitle(host);
			setAlbum(host);
		}

		else {
			setTitle(name);
			setAlbum(name);
		}

		setArtist(host);
	}

	else
	{
		if(name.isEmpty()) {
			setTitle(stationUrl);
			setAlbum(stationUrl);
		} else {
			setTitle(name);
			setAlbum(name);
		}

		setArtist(stationUrl);
	}

	m->albumId = -1;
}

QString MetaData::radioStation() const
{
	return artist();
}

QString MetaData::radioStationName() const
{
	return album();
}

bool MetaData::hasAlbumArtist() const
{
	return (m->albumArtistIdx > 0);
}

QString MetaData::toString() const
{
	QStringList lst;
	lst << m->title;
	lst << "by " << this->artist() << " (" << albumArtist() << ")";
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

QHash<GenreID, Genre>& MetaData::genrePool() const
{
	static QHash<GenreID, Genre> pool;
	return pool;
}


const Util::Set<GenreID>& MetaData::genreIds() const
{
	return m->genres;
}

Util::Set<Genre> MetaData::genres() const
{
	Util::Set<Genre> genres;

	for(GenreID genre_id : m->genres){
		genres.insert( genrePool().value(genre_id) );
	}

	return genres;
}

void MetaData::setGenres(const Util::Set<Genre>& genres)
{
	m->genres.clear();
	for(const Genre& genre : genres)
	{
		GenreID id = genre.id();
		if(!genrePool().contains(id))
		{
			genrePool().insert(id, genre);
		}

		m->genres << id;
	}
}

bool MetaData::hasGenre(const Genre& genre) const
{
	for(const GenreID& id : m->genres)
	{
		if(id == genre.id()){
			return true;
		}
	}

	return false;
}

bool MetaData::removeGenre(const Genre& genre)
{
	m->genres.remove(genre.id());
	return true;
}

bool MetaData::addGenre(const Genre& genre)
{
	GenreID id = genre.id();
	if(!genrePool().contains(id))
	{
		genrePool().insert(id, genre);
	}

	m->genres << id;

	return true;
}

void MetaData::setGenres(const QStringList& new_genres)
{
	m->genres.clear();
	for(const QString& g : new_genres)
	{
		Genre genre(g);
		addGenre(genre);
	}
}

QString MetaData::genresToString() const
{
	return genresToList().join(",");
}

QStringList MetaData::genresToList() const
{
	QStringList new_genres;
	for(const GenreID& id : m->genres)
	{
		new_genres << genrePool().value(id).name();
	}

	return new_genres;
}

QString MetaData::filepath() const
{
	return m->filepath;
}

QString MetaData::setFilepath(QString filepath, RadioMode mode)
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

	if(is_local_path)
	{
		QDir dir(filepath);
		m->filepath = dir.absolutePath();

		if(mode == RadioMode::Undefined)
		{
			mode = RadioMode::Off;
		}
	}

	else if(filepath.contains("soundcloud.com"))
	{
		m->filepath = filepath;

		if(mode == RadioMode::Undefined)
		{
			mode = RadioMode::Soundcloud;
		}
	}

	else
	{
		m->filepath = filepath;

		if(mode == RadioMode::Undefined)
		{
			mode = RadioMode::Station;
		}
	}

	m->radioMode = mode;

	return m->filepath;
}

RadioMode MetaData::radioMode() const
{
	return m->radioMode;
}

void MetaData::changeRadioMode(RadioMode mode)
{
	m->radioMode = mode;
}

bool MetaData::isValid() const
{
	return (!filepath().isEmpty());
}

void MetaData::setCreatedDate(uint64_t t)
{
	m->createdate = t;
}

uint64_t MetaData::createdDate() const
{
	return m->createdate;
}

QDateTime MetaData::createdDateTime() const
{
	return Util::intToDate(m->createdate);
}

void MetaData::setModifiedDate(uint64_t t)
{
	m->modifydate = t;
}

uint64_t MetaData::modifiedDate() const
{
	return m->modifydate;
}

QDateTime MetaData::modifiedDateTime() const
{
	return Util::intToDate(m->modifydate);
}
