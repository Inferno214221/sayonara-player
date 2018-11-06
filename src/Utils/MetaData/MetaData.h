/* MetaData.h */

/* Copyright (C) 2011-2017 Lucio Carreras
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
 *      Author: Lucio Carreras
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

/**
 * @brief The MetaData class
 * @ingroup MetaDataHelper
 */
class MetaData :
	public LibraryItem
{
	PIMPL(MetaData)

public:
	MilliSeconds length_ms;
	Filesize filesize;

	TrackID id;
	ArtistId artist_id;
	AlbumId album_id;

	Bitrate bitrate;
	uint16_t track_num;
	uint16_t year;

	Disc discnumber;
	Disc n_discs;

	LibraryId library_id;
	Rating rating;

	bool played;
	bool is_extern;
	bool pl_playing;
	bool is_disabled;

public:
	MetaData();
	explicit MetaData (const QString& path);
	MetaData(const MetaData& );
	MetaData( MetaData&& );
	MetaData& operator=(const MetaData& md);
	MetaData& operator=(MetaData&& md);

	~MetaData();

	QString title() const;
	void set_title(const QString& title);

	QString artist() const;
	void set_artist(const QString& artist);

	QString album() const;
	void set_album(const QString& album);

	const QString& comment() const;
	void set_comment(const QString& comment);

	QString filepath() const;
	QString set_filepath(QString filepath);

	ArtistId album_artist_id() const;
	QString album_artist() const;
	bool has_album_artist() const;

	void set_album_artist(const QString& album_artist, ArtistId id=-1);
	void set_album_artist_id(ArtistId id);

	void set_radio_station(const QString& name);
	QString radio_station() const;

	RadioMode radio_mode() const;
	bool is_valid() const;

	bool operator==(const MetaData& md) const;
	bool operator!=(const MetaData& md) const;
	bool is_equal(const MetaData& md) const;
	bool is_equal_deep(const MetaData& md) const;

	const Util::Set<GenreID>& genre_ids() const;
	Util::Set<Genre> genres() const;
	bool has_genre(const Genre& genre) const;
	bool remove_genre(const Genre& genre);
	bool add_genre(const Genre& genre);
	void set_genres(const Util::Set<Genre>& genres);
	void set_genres(const QStringList& genres);

	QString genres_to_string() const;
	QStringList genres_to_list() const;

	QString to_string() const;

	static QVariant toVariant(const MetaData& md);
	static bool fromVariant(const QVariant& v, MetaData& md);

private:
	QHash<GenreID, Genre>& genre_pool() const;
};

#ifndef MetaDataDeclared
	Q_DECLARE_METATYPE(MetaData)
	#define MetaDataDeclared
#endif

#endif /* METADATA_H_ */
