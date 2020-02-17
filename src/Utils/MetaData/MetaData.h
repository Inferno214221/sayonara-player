/* MetaData.h */

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

/*
 * MetaData.h
 *
 *  Created on: Mar 10, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef METADATA_H_
#define METADATA_H_

#include "Utils/MetaData/LibraryItem.h"
#include "Utils/MetaData/RadioMode.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Pimpl.h"

#include <QMetaType>
#include <QString>

class QDateTime;
/**
 * @brief The MetaData class
 * @ingroup MetaDataHelper
 */
class MetaData :
	public LibraryItem
{
	PIMPL(MetaData)

public:
	MetaData();
	explicit MetaData (const QString& path);
	MetaData(const MetaData& other);
	MetaData(MetaData&& other) noexcept;
	MetaData& operator=(const MetaData& md);
	MetaData& operator=(MetaData&& md) noexcept;

	~MetaData();

	QString title() const;
	void setTitle(const QString& title);

	QString artist() const;
	void setArtist(const QString& artist);
	ArtistId artistId() const;
	void setArtistId(ArtistId id);

	QString album() const;
	void setAlbum(const QString& album);
	AlbumId albumId() const;
	void setAlbumId(AlbumId id);

	const QString& comment() const;
	void setComment(const QString& comment);

	QString filepath() const;
	QString setFilepath(QString filepath, RadioMode mode=RadioMode::Undefined);

	ArtistId albumArtistId() const;
	QString albumArtist() const;
	bool hasAlbumArtist() const;

	void setAlbumArtist(const QString& albumArtist, ArtistId id=-1);
	void setAlbumArtistId(ArtistId id);

	void setRadioStation(const QString& name);
	QString radioStation() const;

	RadioMode radioMode() const;
	void changeRadioMode(RadioMode mode);

	bool isValid() const;

	bool operator==(const MetaData& md) const;
	bool operator!=(const MetaData& md) const;
	bool isEqual(const MetaData& md) const;
	bool isEqualDeep(const MetaData& md) const;

	const Util::Set<GenreID>& genreIds() const;
	Util::Set<Genre> genres() const;
	bool hasGenre(const Genre& genre) const;
	bool removeGenre(const Genre& genre);
	bool addGenre(const Genre& genre);
	void setGenres(const Util::Set<Genre>& genres);
	void setGenres(const QStringList& genres);

	void setCreatedDate(uint64_t t);
	uint64_t createdDate() const;
	QDateTime createdDateTime() const;

	void setModifiedDate(uint64_t t);
	uint64_t modifiedDate() const;
	QDateTime modifiedDateTime() const;

	QString genresToString() const;
	QStringList genresToList() const;

	QString toString() const;

	static QVariant toVariant(const MetaData& md);
	static bool fromVariant(const QVariant& v, MetaData& md);

	Disc discnumber() const;
	void setDiscnumber(const Disc& value);

	Disc discCount() const;
	void setDiscCount(const Disc& value);

	Bitrate bitrate() const;
	void setBitrate(const Bitrate& value);

	TrackNum trackNumber() const;
	void setTrackNumber(const uint16_t& value);

	Year year() const;
	void setYear(const uint16_t& value);

	Filesize filesize() const;
	void setFilesize(const Filesize& value);

	Rating rating() const;
	void setRating(const Rating& value);

	MilliSeconds durationMs() const;
	void setDurationMs(const MilliSeconds& value);

	bool isExtern() const;
	void setExtern(bool value);

	bool isDisabled() const;
	void setDisabled(bool value);

	LibraryId libraryId() const;
	void setLibraryid(const LibraryId& value);

	TrackID id() const;
	void setId(const TrackID& value);

private:
	QHash<GenreID, Genre>& genrePool() const;
};

#ifndef MetaDataDeclared
Q_DECLARE_METATYPE(MetaData)
	#define MetaDataDeclared
#endif

#endif /* METADATA_H_ */
