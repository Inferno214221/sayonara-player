
/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "ColumnHeader.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

#include <QAction>

#include <algorithm>

using namespace Library;

struct ColumnHeader::Private
{
	int preferredSize;
	int defaultSize;

	QMap<Qt::SortOrder, SortOrder> sortorderMap;

	ColumnIndex::IntegerType columnIndex;

	bool switchable;
	bool stretchable;

	Private(ColumnIndex::IntegerType columnIndex, bool switchable, SortOrder sortorderAscending,
	        SortOrder sortorderDescending, int sz, bool stretchable) :
		preferredSize(sz),
		defaultSize(sz),
		columnIndex(columnIndex),
		switchable(switchable),
		stretchable(stretchable)
	{
		sortorderMap[Qt::SortOrder::AscendingOrder] = sortorderAscending;
		sortorderMap[Qt::SortOrder::DescendingOrder] = sortorderDescending;
	}
};

ColumnHeader::~ColumnHeader() = default;

ColumnHeader::ColumnHeader(ColumnIndex::IntegerType type, bool switchable, SortOrder sortorderAscending,
                           SortOrder sortorderDescending, int preferredSize, bool stretchable)
{
	m = Pimpl::make<Private>(type, switchable, sortorderAscending, sortorderDescending, preferredSize, stretchable);
}

QString ColumnHeader::hash() const
{
	return hashPrefix() + QString::number(int(m->columnIndex));
}

bool ColumnHeader::isSwitchable() const
{
	return m->switchable;
}

bool ColumnHeader::isStretchable() const
{
	return m->stretchable;
}

int ColumnHeader::defaultSize() const
{
	return m->defaultSize;
}

SortOrder ColumnHeader::sortorder(Qt::SortOrder qtSortorder) const
{
	return m->sortorderMap[qtSortorder];
}

ColumnIndex::IntegerType ColumnHeader::columnIndex() const
{
	return m->columnIndex;
}

ColumnHeaderTrack::ColumnHeaderTrack(ColumnIndex::Track type, bool switchable, SortOrder sortAsc, SortOrder sortDesc,
                                     int preferredWidth, bool stretchable) :
	ColumnHeader(ColumnIndex::IntegerType(type), switchable, sortAsc, sortDesc, preferredWidth, stretchable) {}

QString ColumnHeaderTrack::title() const
{
	ColumnIndex::Track type = ColumnIndex::Track(ColumnHeader::columnIndex());
	switch(type)
	{
		case ColumnIndex::Track::TrackNumber:
			return "#";
		case ColumnIndex::Track::Title:
			return Lang::get(Lang::Title).toFirstUpper();
		case ColumnIndex::Track::Artist:
			return Lang::get(Lang::Artist).toFirstUpper();
		case ColumnIndex::Track::Album:
			return Lang::get(Lang::Album).toFirstUpper();
		case ColumnIndex::Track::Discnumber:
			return Lang::get(Lang::Disc);
		case ColumnIndex::Track::Year:
			return Lang::get(Lang::Year).toFirstUpper();
		case ColumnIndex::Track::Length:
			return Lang::get(Lang::DurationShort).toFirstUpper();
		case ColumnIndex::Track::Bitrate:
			return Lang::get(Lang::Bitrate).toFirstUpper();
		case ColumnIndex::Track::Filesize:
			return Lang::get(Lang::Filesize).toFirstUpper();
		case ColumnIndex::Track::Filetype:
			return Lang::get(Lang::Filetype).toFirstUpper();
		case ColumnIndex::Track::AddedDate:
			return Lang::get(Lang::Created).toFirstUpper();
		case ColumnIndex::Track::ModifiedDate:
			return Lang::get(Lang::Modified).toFirstUpper();
		case ColumnIndex::Track::Rating:
			return Lang::get(Lang::Rating).toFirstUpper();
		default:
			return QString();
	}
}

QString ColumnHeaderTrack::hashPrefix() const
{
	return "track";
}

ColumnHeaderAlbum::ColumnHeaderAlbum(ColumnIndex::Album type, bool switchable, SortOrder sortAsc, SortOrder sortDesc,
                                     int preferredWidth, bool stretchable) :
	ColumnHeader(ColumnIndex::IntegerType(type), switchable, sortAsc, sortDesc, preferredWidth, stretchable) {}

QString ColumnHeaderAlbum::title() const
{
	auto type = ColumnIndex::Album(ColumnHeader::columnIndex());
	switch(type)
	{
		case ColumnIndex::Album::MultiDisc:
			return "#";
		case ColumnIndex::Album::Name:
			return Lang::get(Lang::Album).toFirstUpper();
//		case ColumnIndex::Album::AlbumArtist:
//			return Lang::get(Lang::AlbumArtists).toFirstUpper();
		case ColumnIndex::Album::Duration:
			return Lang::get(Lang::DurationShort).toFirstUpper();
		case ColumnIndex::Album::NumSongs:
			return Lang::get(Lang::NumTracks).toFirstUpper();
		case ColumnIndex::Album::Year:
			return Lang::get(Lang::Year).toFirstUpper();
		case ColumnIndex::Album::Rating:
			return Lang::get(Lang::Rating).toFirstUpper();
		default:
			return QString();
	}
}

QString ColumnHeaderAlbum::hashPrefix() const
{
	return "album";
}

ColumnHeaderArtist::ColumnHeaderArtist(ColumnIndex::Artist type, bool switchable, SortOrder sortAsc, SortOrder sortDesc,
                                       int preferredWidth, bool stretchable) :
	ColumnHeader(ColumnIndex::IntegerType(type), switchable, sortAsc, sortDesc, preferredWidth, stretchable) {}

QString ColumnHeaderArtist::title() const
{
	auto type = ColumnIndex::Artist(ColumnHeader::columnIndex());
	switch(type)
	{
		case ColumnIndex::Artist::Name:
			if(GetSetting(Set::Lib_ShowAlbumArtists))
			{
				return Lang::get(Lang::AlbumArtist);
			}

			else
			{
				return Lang::get(Lang::Artist);
			}
		case ColumnIndex::Artist::Tracks:
			return Lang::get(Lang::NumTracks).toFirstUpper();
		default:
			return QString();
	}
}

QString ColumnHeaderArtist::hashPrefix() const
{
	return "artist";
}

