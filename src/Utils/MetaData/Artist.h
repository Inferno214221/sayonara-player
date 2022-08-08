/* Artist.h */

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

#ifndef SAYONARA_ARTIST_H_
#define SAYONARA_ARTIST_H_

#include "Utils/MetaData/LibraryItem.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Pimpl.h"

#include <QStringList>
#include <QMetaType>
#include <deque>

class Artist :
	public LibraryItem
{
	PIMPL(Artist)

	public:
		Artist();
		Artist(const Artist& other);
		Artist(Artist&& other) noexcept;

		Artist& operator=(const Artist& other);
		Artist& operator=(Artist&& other) noexcept;

		~Artist();

		QString name() const;
		void setName(const QString& name);

		static bool fromVariant(const QVariant& v, Artist& a);
		static QVariant toVariant(const Artist& a);
		void print() const;

		uint16_t albumcount() const;
		void setAlbumcount(const uint16_t& value);

		uint16_t songcount() const;
		void setSongcount(const uint16_t& value);

		ArtistId id() const;
		void setId(const ArtistId& value);
};

Q_DECLARE_METATYPE(Artist)

class ArtistList :
	public std::deque<Artist>
{
		using Parent = std::deque<Artist>;

	public:
		using Size = Parent::size_type;

		ArtistList();
		~ArtistList();

		static QString majorArtist(const QStringList& artists);

		QString majorArtist() const;

		Artist first() const;
		bool contains(ArtistId artistId) const;
		int count() const;

		ArtistList& operator<<(const Artist& artist);
		ArtistList& appendUnique(const ArtistList& other);
};

#endif
