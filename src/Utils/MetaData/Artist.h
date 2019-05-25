/* Artist.h */

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

#ifndef _ARTIST_H_
#define _ARTIST_H_

#include "Utils/MetaData/LibraryItem.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Pimpl.h"

#include <QStringList>
#include <QMetaType>
#include <deque>

/**
 * @brief The Artist class
 * @ingroup MetaDataHelper
 */
class Artist :
		public LibraryItem
{
	PIMPL(Artist)

public:
	ArtistId id;
	uint16_t num_albums;
	uint16_t num_songs;

	Artist();
	Artist(const Artist& other);
	Artist(Artist&& other);

	Artist& operator=(const Artist& other);
	Artist& operator=(Artist&& other);

	~Artist();

	QString name() const;
	void set_name(const QString& name);

	static bool fromVariant(const QVariant& v, Artist& a);
	static QVariant toVariant(const Artist& a);
	void print() const ;
};


Q_DECLARE_METATYPE(Artist)

/**
 * @brief ArtistList
 * @ingroup MetaDataHelper
 */
class ArtistList :
		public std::deque<Artist>
{
	using Parent=std::deque<Artist>;

public:
	ArtistList();
	~ArtistList();

	/**
	 * @brief extract the main artist out of the artist list
	 * @param artists artist list
	 * @return the name that appears more than 2/3 of all available artists
	 */
	static QString get_major_artist(const QStringList& artists);

	/**
	 * @brief extract the main artist out of the artist list
	 * @param artists artist list
	 * @return the name that appears more than 2/3 of all available artists
	 */
	QString get_major_artist() const;


	bool contains(ArtistId artist_id) const;

	int count() const;
	ArtistList& operator <<(const Artist& artist);
	Artist first() const;

	ArtistList& append_unique(const ArtistList& other);

	void sort(Library::SortOrder so);
};

#endif
