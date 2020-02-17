/* Album.h */

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
	Album();
	Album(const Album& other);
	Album(Album&& other) noexcept ;

	Album& operator=(const Album& other);
	Album& operator=(Album&& other) noexcept;
	bool operator==(const Album& other) const;

	~Album();

	AlbumId id() const;
	void setId(const AlbumId& id);

	QString name() const;
	void setName(const QString& name);

	QStringList artists() const;
	void setArtists(const QStringList& artists);

	QStringList albumArtists() const;
	void setAlbumArtists(const QStringList& albumArtists);

	QStringList pathHint() const;
	void setPathHint(const QStringList& paths);

	Seconds durationSec() const;
	void setDurationSec(const Seconds& sec);

	TrackNum songcount() const;
	void setSongcount(const TrackNum& songs);

	Year year() const;
	void setYear(const Year& year);

	Disc disccount() const;

	Rating rating() const;
	void setRating(const Rating& rating);

	bool isSampler() const;

	QList<Disc> discnumbers() const;
	void setDiscnumbers(const QList<Disc>& discnumbers);

	static QVariant toVariant(const Album& album);
	static bool fromVariant(const QVariant& v, Album& album);
	QString toString() const;
};


/**
 * @brief The AlbumList class
 * @ingroup MetaDataHelper
 */
class AlbumList : public std::deque<Album>
{
	using Parent=std::deque<Album>;

public:
	bool contains(AlbumId albumId) const;

	int count() const;
	AlbumList& operator <<(const Album& album);
	Album first() const;
	Album& operator[](int idx);
	const Album& operator[](int idx) const;

	AlbumList& appendUnique(const AlbumList& other);
	AlbumList& appendUnique(AlbumList&& other) noexcept;

	void sort(::Library::SortOrder so);
};

#endif //HEADER_ALBUM_H_


