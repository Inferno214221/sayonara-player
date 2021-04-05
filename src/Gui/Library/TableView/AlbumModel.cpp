/* LibraryItemModelAlbums.cpp */

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
 * LibraryItemModelAlbums.cpp
 *
 *  Created on: Apr 26, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "AlbumModel.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Tagging/UserTaggingOperations.h"

#include "Gui/Library/Header/ColumnIndex.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"

#include <QPixmap>
#include <QColor>

using namespace Library;

struct AlbumModel::Private
{
	QPixmap pixmapSingle;
	QPixmap pixmapMulti;
	QPair<int, Rating> tempRating;
	Tagging::UserOperations* uto = nullptr;

	Private() :
		pixmapSingle(Gui::Util::pixmap(QStringLiteral("cd.png"), Gui::Util::NoTheme)),
		pixmapMulti(Gui::Util::pixmap(QStringLiteral("cds.png"), Gui::Util::NoTheme)),
		tempRating {-1, Rating::Zero} {}
};

AlbumModel::AlbumModel(QObject* parent, AbstractLibrary* library) :
    ItemModel(+ColumnIndex::Album::Count, parent, library)
{
	m = Pimpl::make<AlbumModel::Private>();

	connect(library, &AbstractLibrary::sigCurrentAlbumChanged, this, &AlbumModel::albumChanged);
}

AlbumModel::~AlbumModel() = default;

Id AlbumModel::mapIndexToId(int index) const
{
	const auto& albums = library()->albums();
	return (Util::between(index, albums))
	       ? albums[index].id()
	       : -1;
}

QString AlbumModel::searchableString(int row) const
{
	const auto& albums = library()->albums();
	return (Util::between(row, albums))
	       ? albums[row].name()
	       : QString {};
}

Cover::Location AlbumModel::cover(const QModelIndexList& indexes) const
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

	const auto row = static_cast<AlbumList::size_type>(rows.first());
	const auto& albums = library()->albums();

	return (Util::between(row, albums))
	       ? Cover::Location::coverLocation(albums[row])
	       : Cover::Location::invalidLocation();
}

QVariant AlbumModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	const auto& albums = library()->albums();
	if(index.row() >= albums.count())
	{
		return QVariant();
	}

	const auto row = index.row();
	const auto column = index.column();
	const auto col = ColumnIndex::Album(column);

	const auto& album = albums[row];

	if(role == Qt::TextAlignmentRole)
	{
		return (col == ColumnIndex::Album::Name)
		       ? (+Qt::AlignVCenter | +Qt::AlignLeft)
		       : (+Qt::AlignVCenter | +Qt::AlignRight);
	}

	else if(role == Qt::ForegroundRole)
	{
		if(col == ColumnIndex::Album::MultiDisc)
		{
			return QColor(0, 0, 0);
		}
	}

	else if(role == Qt::DecorationRole)
	{
		if(col == ColumnIndex::Album::MultiDisc)
		{
			return (album.discnumbers().size() > 1)
			       ? m->pixmapMulti
			       : m->pixmapSingle;
		}
	}

	else if(role == Qt::DisplayRole || role == Qt::EditRole)
	{
		switch(col)
		{
			case ColumnIndex::Album::NumSongs:
				return QString::number(album.songcount());

			case ColumnIndex::Album::Year:
				return (album.year() == 0)
				       ? Lang::get(Lang::UnknownYear)
				       : QVariant::fromValue(album.year());

			case ColumnIndex::Album::Name:
				return (album.name().trimmed().isEmpty())
				       ? Lang::get(Lang::UnknownAlbum)
				       : album.name();

			case ColumnIndex::Album::Duration:
				return ::Util::msToString(album.durationSec() * 1000, "$He $M:$S");

			case ColumnIndex::Album::Rating:
			{
				if(role == Qt::DisplayRole)
				{
					return QVariant();
				}

				const auto rating = (row == m->tempRating.first)
				                    ? m->tempRating.second
				                    : album.rating();

				return QVariant::fromValue(rating);
			}

			default:
				return QVariant();
		}
	}

	else if(role == Qt::SizeHintRole)
	{
		return QSize(1, Gui::Util::viewRowHeight());
	}

	return QVariant();
}

bool AlbumModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if((index.column() != int(ColumnIndex::Album::Rating) ||
	    (role != Qt::EditRole)))
	{
		return false;
	}

	const auto row = index.row();

	const auto& albums = library()->albums();
	if(Util::between(row, albums))
	{
		const auto& album = albums[row];
		const auto rating = value.value<Rating>();

		if(album.rating() != rating)
		{
			m->tempRating.first = row;
			m->tempRating.second = rating;

			if(!m->uto)
			{
				m->uto = new Tagging::UserOperations(-1, this);
			}

			m->uto->setAlbumRating(album, rating);
		}
	}

	return false;
}

void AlbumModel::albumChanged(int row)
{
	m->tempRating.first = -1;
	emit dataChanged(this->index(row, 0), this->index(row, columnCount()));
}

int AlbumModel::rowCount(const QModelIndex&) const
{
	return library()->albums().count();
}

Qt::ItemFlags AlbumModel::flags(const QModelIndex& index) const
{
	if(!index.isValid())
	{
		return Qt::ItemIsEnabled;
	}

	return (index.column() == +ColumnIndex::Album::Rating)
	       ? (ItemModel::flags(index) | Qt::ItemIsEditable)
	       : ItemModel::flags(index);
}

int AlbumModel::searchableColumn() const
{
	return +ColumnIndex::Album::Name;
}

const MetaDataList& Library::AlbumModel::selectedMetadata() const
{
	return library()->tracks();
}
