/* LibraryItemModelTracks.cpp */

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
 * LibraryItemModelTracks.cpp
	 *
 *  Created on: Apr 24, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "TrackModel.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Tagging/UserTaggingOperations.h"

#include "Gui/Library/Header/ColumnIndex.h"
#include "Gui/Utils/GuiUtils.h"

#include "Utils/Algorithm.h"
#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"

#include <QSize>
#include <QPair>
#include <QDateTime>
#include <QLocale>

using namespace Library;

struct TrackModel::Private
{
	Tagging::UserOperations* uto {nullptr};
	QLocale locale {};
};

TrackModel::TrackModel(QObject* parent, AbstractLibrary* library) :
	ItemModel(parent, library)
{
	m = Pimpl::make<Private>();

	connect(library,
	        &AbstractLibrary::sigCurrentTrackChanged,
	        this,
	        &TrackModel::trackMetaDataChanged);
	ListenSetting(Set::Player_Language, TrackModel::languageChanged);
}

TrackModel::~TrackModel() = default;

QVariant TrackModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	if(index.row() >= library()->tracks().count())
	{
		return QVariant();
	}

	const auto indexColumn = ColumnIndex::Track(index.column());

	if(role == Qt::TextAlignmentRole)
	{
		const QMap<ColumnIndex::Track, Qt::AlignmentFlag> alignMap {
			{ColumnIndex::Track::TrackNumber,  Qt::AlignRight},
			{ColumnIndex::Track::Bitrate,      Qt::AlignRight},
			{ColumnIndex::Track::Length,       Qt::AlignRight},
			{ColumnIndex::Track::Filesize,     Qt::AlignRight},
			{ColumnIndex::Track::Discnumber,   Qt::AlignRight},
			{ColumnIndex::Track::Filetype,     Qt::AlignCenter},
			{ColumnIndex::Track::Year,         Qt::AlignCenter},
			{ColumnIndex::Track::ModifiedDate, Qt::AlignCenter},
			{ColumnIndex::Track::AddedDate,    Qt::AlignCenter}
		};

		const auto alignment = alignMap.contains(indexColumn) ? alignMap[indexColumn]
		                                                      : Qt::AlignLeft;

		return QVariant::fromValue(static_cast<int>(Qt::AlignVCenter | alignment));
	}

	else if(role == Qt::DisplayRole || role == Qt::EditRole)
	{
		const auto& track = library()->tracks().at(index.row());

		switch(indexColumn)
		{
			case ColumnIndex::Track::TrackNumber:
				return QVariant(track.trackNumber());

			case ColumnIndex::Track::Title:
				return QVariant(track.title());

			case ColumnIndex::Track::Artist:
				return QVariant(track.artist());

			case ColumnIndex::Track::Length:
				return QVariant(::Util::msToString(track.durationMs(),
				                                   "$He $M:$S"));

			case ColumnIndex::Track::Album:
				return QVariant(track.album());

			case ColumnIndex::Track::Discnumber:
				return QVariant(QString::number(track.discnumber()));

			case ColumnIndex::Track::Year:
				return (track.year() == 0) ?
				       QVariant(Lang::get(Lang::UnknownYear)) :
				       QVariant(track.year());

			case ColumnIndex::Track::Bitrate:
			{
				const auto bitrate = (track.bitrate() / 1000);
				return (bitrate == 0) ?
				       "-" :
				       QString("%1 %2").arg(bitrate).arg(tr("kBit/s"));
			}

			case ColumnIndex::Track::Filesize:
				return ::Util::File::getFilesizeString(track.filesize());

			case ColumnIndex::Track::Filetype:
			{
				const auto extension = ::Util::File::getFileExtension(track.filepath());
				return (extension.isEmpty()) ? "-" : extension;
			}

			case ColumnIndex::Track::AddedDate:
			{
				const auto format = m->locale.dateFormat(QLocale::ShortFormat);
				return track.createdDateTime().date().toString(format);
			}

			case ColumnIndex::Track::ModifiedDate:
			{
				const auto format = m->locale.dateFormat(QLocale::ShortFormat);
				return track.modifiedDateTime().date().toString(format);
			}

			case ColumnIndex::Track::Rating:
			{
				if(role == Qt::DisplayRole)
				{
					return QVariant();
				}

				if(m->uto && m->uto->newRating(track.id()) != Rating::Last)
				{
					return QVariant::fromValue(m->uto->newRating(track.id()));
				}

				return QVariant::fromValue(track.rating());
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

Qt::ItemFlags TrackModel::flags(const QModelIndex& index) const
{
	if(!index.isValid())
	{
		return Qt::ItemIsEnabled;
	}

	return (index.column() == +ColumnIndex::Track::Rating)
	       ? (QAbstractTableModel::flags(index) | Qt::ItemIsEditable)
	       : QAbstractTableModel::flags(index);
}

bool
TrackModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if((index.column() != +ColumnIndex::Track::Rating) ||
	   (role != Qt::EditRole))
	{
		return false;
	}

	const auto row = index.row();
	const auto& tracks = library()->tracks();

	if(Util::between(row, tracks))
	{
		const auto& track = tracks[row];
		const auto rating = value.value<Rating>();

		if(track.rating() != rating)
		{
			if(!m->uto)
			{
				m->uto = new Tagging::UserOperations(-1, this);
			}

			m->uto->setTrackRating(track, rating);

			emit dataChanged(
				this->index(row, +ColumnIndex::Track::Rating),
				this->index(row, +ColumnIndex::Track::Rating)
			);

			return true;
		}
	}

	return false;
}

void TrackModel::trackMetaDataChanged(int row)
{
	emit dataChanged(this->index(row, 0), this->index(row, columnCount()));
}

int TrackModel::rowCount(const QModelIndex&) const
{
	return library()->tracks().count();
}

Id TrackModel::mapIndexToId(int row) const
{
	const auto& tracks = library()->tracks();
	return Util::between(row, tracks) ? tracks[row].id() : -1;
}

QString TrackModel::searchableString(int row) const
{
	const auto& tracks = library()->tracks();
	return Util::between(row, tracks) ? tracks[row].title() : QString();
}

Cover::Location TrackModel::cover(const QModelIndexList& indexes) const
{
	const auto& tracks = library()->tracks();

	Util::Set<int> rows;
	for(const auto& index : indexes)
	{
		const auto row = index.row();
		if(Util::between(row, tracks))
		{
			rows.insert(row);
		}
	}

	if(rows.isEmpty())
	{
		return Cover::Location::invalidLocation();
	}

	const auto firstRow = rows.first();
	const auto albumId = tracks[firstRow].albumId();

	const auto containsMultipleAlbums = Util::Algorithm::contains(rows, [&](const auto row) {
		return (tracks[row].albumId() != albumId);
	});

	return containsMultipleAlbums
	       ? Cover::Location::invalidLocation()
	       : Cover::Location::coverLocation(tracks[firstRow]);
}

int TrackModel::searchableColumn() const
{
	return +ColumnIndex::Track::Title;
}

const MetaDataList& Library::TrackModel::selectedMetadata() const
{
	return library()->currentTracks();
}

void TrackModel::languageChanged()
{
	m->locale = Util::Language::getCurrentLocale();
}
