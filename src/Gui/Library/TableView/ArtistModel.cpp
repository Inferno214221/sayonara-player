/* LibraryItemModelArtists.cpp */

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
 * LibraryItemModelArtists.cpp
 *
 *  Created on: Apr 26, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "ArtistModel.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Library/AbstractLibrary.h"

#include "Gui/Library/Header/ColumnIndex.h"
#include "Gui/Utils/GuiUtils.h"

#include "Utils/MetaData/Artist.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"
#include "Utils/globals.h"

#include <QPixmap>

using namespace Library;

ArtistModel::ArtistModel(QObject* parent, AbstractLibrary* library) :
	ItemModel(parent, library) {}

ArtistModel::~ArtistModel() = default;

Id ArtistModel::mapIndexToId(int row) const
{
	const auto& artists = library()->artists();
	return Util::between(row, artists)
	       ? artists[ArtistList::Size(row)].id()
	       : -1;
}

QString ArtistModel::searchableString(int row) const
{
	const auto& artists = library()->artists();
	return Util::between(row, artists)
	       ? artists[ArtistList::Size(row)].name()
	       : QString();
}

QVariant ArtistModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	const ArtistList& artists = library()->artists();
	if(index.row() >= artists.count())
	{
		return QVariant();
	}

	const auto row = index.row();
	const auto col = index.column();
	auto columnIndex = ColumnIndex::Artist(col);

	const Artist& artist = artists[ArtistList::Size(row)];

	if(role == +Qt::TextAlignmentRole)
	{
		switch(columnIndex)
		{
			case ColumnIndex::Artist::Name:
				return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
			default:
				return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
		}
	}

	else if(role == +Qt::DisplayRole)
	{
		switch(columnIndex)
		{
			case ColumnIndex::Artist::Name:
				return (artist.name().isEmpty())
				       ? Lang::get(Lang::UnknownArtist)
				       : artist.name();

			case ColumnIndex::Artist::Tracks:
				return QString::number(artist.songcount());

			default:
				return QVariant();
		}
	}

	else if(role == +Qt::SizeHintRole)
	{
		return QSize(1, Gui::Util::viewRowHeight());
	}

	return QVariant();
}

int ArtistModel::rowCount(const QModelIndex&) const
{
	return library()->artists().count();
}

Qt::ItemFlags ArtistModel::flags(const QModelIndex& index) const
{
	return index.isValid()
	       ? QAbstractTableModel::flags(index)
	       : Qt::ItemIsEnabled;
}

Cover::Location ArtistModel::cover(const QModelIndexList& indexes) const
{
	Util::Set<int> rows;
	for(const auto& index : indexes)
	{
		rows.insert(index.row());
	}

	if(rows.size() != 1)
	{
		return Cover::Location::invalidLocation();
	}

	const auto& artists = library()->artists();
	const auto row = static_cast<ArtistList::Size>(rows.first());

	return (Util::between(row, artists))
	       ? Cover::Location::coverLocation(artists[row])
	       : Cover::Location::invalidLocation();
}

int ArtistModel::searchableColumn() const
{
	return static_cast<int>(ColumnIndex::Artist::Name);
}

const MetaDataList& Library::ArtistModel::selectedMetadata() const
{
	return library()->tracks();
}
