/* Album.h */

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

#ifndef HEADER_ALBUM_H_
#define HEADER_ALBUM_H_

#include "Utils/MetaData/LibraryItem.h"
#include "Utils/Library/Sortorder.h"
#include <QMetaType>
#include <deque>

class QVariant;
class QStringList;
class Album;

Q_DECLARE_METATYPE(Album)

/**
 * @brief The Album class
 * @ingroup MetaDataHelper
 */
class Album :
		public LibraryItem
{
	PIMPL(Album)

public:
	QList<Disc>	discnumbers;

	AlbumId		id;
	Seconds length_sec;
	uint16_t	num_songs;
	uint16_t	year;

	Disc		n_discs;
	Rating		rating;
	bool		is_sampler;


public:
	Album();
	Album(const Album& other);
	Album(Album&& other);

	Album& operator=(const Album& other);
	Album& operator=(Album&& other);

	~Album();

	QString name() const;
	void set_name(const QString& name);

	QStringList artists() const;
	void set_artists(const QStringList& artists);

	QStringList album_artists() const;
	void set_album_artists(const QStringList& album_artists);

	QStringList path_hint() const;
	void set_path_hint(const QStringList& paths);

	static QVariant toVariant(const Album& album);
	static bool fromVariant(const QVariant& v, Album& album);
	QString to_string() const;
};


/**
 * @brief The AlbumList class
 * @ingroup MetaDataHelper
 */
class AlbumList : public std::deque<Album>
{
	using Parent=std::deque<Album>;

public:
	bool contains(AlbumId album_id) const;

	int count() const;
	AlbumList& operator <<(const Album& album);
	Album first() const;
	Album& operator[](int idx);
	const Album& operator[](int idx) const;

	AlbumList& append_unique(const AlbumList& other);
	AlbumList& append_unique(AlbumList&& other);

	void sort(::Library::SortOrder so);
};

#endif //HEADER_ALBUM_H_


