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
#include "Utils/FileUtils.h"
#include "Utils/Algorithm.h"

#include <QDir>
#include <QUrl>
#include <QVariant>
#include <QStringList>
#include <QHash>
#include <QGlobalStatic>
#include <QDateTime>

namespace
{
	QHash<GenreID, Genre> GenrePool; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
}

struct MetaData::Private
{
	QString title;
	QString comment;
	QString filepath;
	Util::Set<GenreID> genres;
	uint64_t createdate {0};
	uint64_t modifydate {0};
	MilliSeconds durationMs {0};
	Filesize filesize {0};
	TrackID id {-1};
	LibraryId libraryId {-1};
	ArtistId artistId {-1};
	AlbumId albumId {-1};
	ArtistId albumArtistId {-1};
	HashValue albumArtistIdx {0};
	HashValue albumIdx {0};
	HashValue artistIdx {0};
	Bitrate bitrate {0};
	TrackNum tracknum {0};
	Year year {0};
	Disc discnumber {0};
	Disc discCount {1};
	Rating rating {Rating::Zero};
	RadioMode radioMode {RadioMode::Off};
	bool isExtern {false};
	bool isDisabled {false};

	Private() = default;
	Private(const Private& other) = default;
	Private(Private&& other) noexcept = default;
	Private& operator=(const Private& other) = default;
	Private& operator=(Private&& other) noexcept = default;

	~Private()
	{
		genres.clear();
	}

	[[nodiscard]] bool isEqual(const Private& other) const
	{
		return (title == other.title) &&
		       (genres == other.genres) &&
		       (createdate == other.createdate) &&
		       (modifydate == other.modifydate) &&
		       (durationMs == other.durationMs) &&
		       (filesize == other.filesize) &&
		       (id == other.id) &&
		       (libraryId == other.libraryId) &&
		       (artistId == other.artistId) &&
		       (albumId == other.albumId) &&
		       (albumArtistId == other.albumArtistId) &&
		       (albumArtistIdx == other.albumArtistIdx) &&
		       (albumIdx == other.albumIdx) &&
		       (artistIdx == other.artistIdx) &&
		       (comment == other.comment) &&
		       (filepath == other.filepath) &&
		       (bitrate == other.bitrate) &&
		       (tracknum == other.tracknum) &&
		       (year == other.year) &&
		       (discnumber == other.discnumber) &&
		       (discCount == other.discCount) &&
		       (rating == other.rating) &&
		       (radioMode == other.radioMode) &&
		       (isExtern == other.isExtern) &&
		       (isDisabled == other.isDisabled);
	}
};

MetaData::MetaData() :
	m {Pimpl::make<Private>()} {}

MetaData::MetaData(const MetaData& other) :
	LibraryItem(other),
	m {Pimpl::make<Private>(*other.m)} {}

MetaData::MetaData(MetaData&& other) noexcept :
	LibraryItem(std::move(other)),
	m {Pimpl::make<Private>(std::move(*other.m))} {}

MetaData::MetaData(const QString& path) :
	MetaData()
{
	setFilepath(path);
}

MetaData::~MetaData()
{
	m->genres.clear();
}

MetaData& MetaData::operator=(const MetaData& other)
{
	LibraryItem::operator=(other);
	*m = *other.m;

	return *this;
}

MetaData& MetaData::operator=(MetaData&& other) noexcept
{
	LibraryItem::operator=(std::move(other));
	*m = std::move(*other.m); // NOLINT(bugprone-use-after-move)

	return *this;
}

bool MetaData::operator==(const MetaData& other) const { return isEqual(other); }

bool MetaData::operator!=(const MetaData& other) const { return !isEqual(other); }

Disc MetaData::discnumber() const { return m->discnumber; }

void MetaData::setDiscnumber(const Disc& d) { m->discnumber = d; }

Disc MetaData::discCount() const { return m->discCount; }

void MetaData::setDiscCount(const Disc& d) { m->discCount = d; }

Bitrate MetaData::bitrate() const { return m->bitrate; }

void MetaData::setBitrate(const Bitrate& value) { m->bitrate = value; }

TrackNum MetaData::trackNumber() const { return m->tracknum; }

void MetaData::setTrackNumber(const TrackNum& value) { m->tracknum = value; }

Year MetaData::year() const { return m->year; }

void MetaData::setYear(const Year& value) { m->year = value; }

Filesize MetaData::filesize() const { return m->filesize; }

void MetaData::setFilesize(const Filesize& value) { m->filesize = value; }

Rating MetaData::rating() const { return m->rating; }

void MetaData::setRating(const Rating& value) { m->rating = value; }

MilliSeconds MetaData::durationMs() const { return m->durationMs; }

void MetaData::setDurationMs(const MilliSeconds& value) { m->durationMs = value; }

bool MetaData::isExtern() const { return m->isExtern; }

void MetaData::setExtern(bool value) { m->isExtern = value; }

bool MetaData::isDisabled() const { return m->isDisabled; }

void MetaData::setDisabled(bool value) { m->isDisabled = value; }

LibraryId MetaData::libraryId() const { return m->libraryId; }

void MetaData::setLibraryid(const LibraryId& value) { m->libraryId = value; }

TrackID MetaData::id() const { return m->id; }

void MetaData::setId(const TrackID& value) { m->id = value; }

bool MetaData::isEqual(const MetaData& other) const { return Util::File::isSamePath(m->filepath, other.filepath()); }

bool MetaData::isEqualDeep(const MetaData& other) const { return m->isEqual(*other.m); }

QString MetaData::title() const { return m->title; }

void MetaData::setTitle(const QString& title) { m->title = title.trimmed(); }

QString MetaData::artist() const { return artistPool().value(m->artistIdx); }

void MetaData::setArtist(const QString& artist)
{
	const auto hashed = qHash(artist);
	if(!artistPool().contains(hashed))
	{
		artistPool().insert(hashed, artist);
	}

	m->artistIdx = hashed;
}

ArtistId MetaData::artistId() const { return m->artistId; }

void MetaData::setArtistId(const ArtistId id) { m->artistId = id; }

QString MetaData::album() const { return albumPool().value(m->albumIdx); }

void MetaData::setAlbum(const QString& album)
{
	const auto hashed = qHash(album);
	if(!albumPool().contains(hashed))
	{
		albumPool().insert(hashed, album);
	}

	m->albumIdx = hashed;
}

AlbumId MetaData::albumId() const { return m->albumId; }

void MetaData::setAlbumId(const AlbumId id) { m->albumId = id; }

const QString& MetaData::comment() const { return m->comment; }

void MetaData::setComment(const QString& comment) { m->comment = comment; }

ArtistId MetaData::albumArtistId() const
{
	if(m->albumArtistId < 0 || m->albumArtistIdx == 0)
	{
		return m->artistId;
	}

	const auto str = artistPool().value(m->albumArtistIdx);
	return !str.isEmpty()
	       ? m->albumArtistId
	       : m->artistId;
}

QString MetaData::albumArtist() const
{
	const auto str = artistPool().value(m->albumArtistIdx);
	return !str.isEmpty()
	       ? str
	       : artist();
}

void MetaData::setAlbumArtist(const QString& albumArtist, const ArtistId id)
{
	const auto hashed = qHash(albumArtist);
	if(!artistPool().contains(hashed))
	{
		artistPool().insert(hashed, albumArtist);
	}

	m->albumArtistIdx = hashed;
	m->albumArtistId = id;
}

void MetaData::setAlbumArtistId(const ArtistId id) { m->albumArtistId = id; }

void MetaData::setRadioStation(const QString& stationUrl, const QString& name)
{
	const auto url = QUrl(stationUrl);
	auto stationName = name;

	if(url.isValid() && !url.scheme().isEmpty())
	{
		if(stationName.isEmpty())
		{
			stationName = url.host();
		}
	}

	else
	{
		if(stationName.isEmpty())
		{
			stationName = stationUrl;
		}
	}

	setTitle(stationName);
	setAlbum(stationName);
	setArtist(stationUrl);

	m->albumId = -1;
}

QString MetaData::radioStation() const { return artist(); }

QString MetaData::radioStationName() const { return album(); }

const Util::Set<GenreID>& MetaData::genreIds() const { return m->genres; }

Util::Set<Genre> MetaData::genres() const
{
	Util::Set<Genre> genres;

	for(const auto& genreId: m->genres)
	{
		genres.insert(GenrePool.value(genreId));
	}

	return genres;
}

void MetaData::setGenres(const Util::Set<Genre>& genres)
{
	m->genres.clear();
	for(const auto& genre: genres)
	{
		const auto genreId = genre.id();
		if(!GenrePool.contains(genreId))
		{
			GenrePool.insert(genreId, genre);
		}

		m->genres << genreId;
	}
}

bool MetaData::hasGenre(const Genre& genre) const
{
	return m->genres.contains(genre.id());
}

bool MetaData::removeGenre(const Genre& genre)
{
	m->genres.remove(genre.id());
	return true;
}

bool MetaData::addGenre(const Genre& genre)
{
	const auto id = genre.id();
	if(!GenrePool.contains(id))
	{
		GenrePool.insert(id, genre);
	}

	m->genres << id;

	return true;
}

void MetaData::setGenres(const QStringList& newGenres)
{
	m->genres.clear();
	for(const auto& newGenre: newGenres)
	{
		const auto genre = Genre(newGenre);
		addGenre(genre);
	}
}

QString MetaData::genresToString() const { return genresToList().join(","); }

QStringList MetaData::genresToList() const
{
	QStringList newGenres;
	for(const auto& id: m->genres)
	{
		newGenres << GenrePool.value(id).name();
	}

	return newGenres;
}

QString MetaData::filepath() const { return m->filepath; }

QString MetaData::setFilepath(const QString& filepath, RadioMode mode)
{
	auto isLocalPath = false;

#ifdef Q_OS_UNIX
	if(filepath.startsWith("/"))
	{
		isLocalPath = true;
	}
#else
	if(filepath.contains(":\\") || filepath.contains("\\\\")){
		is_local_path = true;
	}
#endif

	if(isLocalPath)
	{
		const auto dir = QDir(filepath);
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

RadioMode MetaData::radioMode() const { return m->radioMode; }

void MetaData::changeRadioMode(RadioMode mode) { m->radioMode = mode; }

bool MetaData::isValid() const { return !filepath().isEmpty(); }

void MetaData::setCreatedDate(const uint64_t t) { m->createdate = t; }

uint64_t MetaData::createdDate() const { return m->createdate; }

QDateTime MetaData::createdDateTime() const { return Util::intToDate(m->createdate); }

void MetaData::setModifiedDate(const uint64_t t) { m->modifydate = t; }

uint64_t MetaData::modifiedDate() const { return m->modifydate; }

QDateTime MetaData::modifiedDateTime() const { return Util::intToDate(m->modifydate); }
