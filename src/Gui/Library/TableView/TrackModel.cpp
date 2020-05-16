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
	QPair<int, Rating>			tmpRating;
	Tagging::UserOperations*	uto=nullptr;
	QLocale						locale;
};

TrackModel::TrackModel(QObject* parent, AbstractLibrary* library) :
	ItemModel(parent, library)
{
	m = Pimpl::make<Private>();

	connect(library, &AbstractLibrary::sigCurrentTrackChanged, this, &TrackModel::trackChanged);
	ListenSetting(Set::Player_Language, TrackModel::languageChanged);
}

TrackModel::~TrackModel() = default;

QVariant TrackModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	int col = index.column();

	if (!index.isValid()) {
		return QVariant();
	}

	const MetaDataList& tracks = library()->tracks();

	if (row >= tracks.count()) {
		return QVariant();
	}

	auto indexColumn = ColumnIndex::Track(col);

	if (role == Qt::TextAlignmentRole)
	{
		int alignment = Qt::AlignVCenter;

		if (indexColumn == ColumnIndex::Track::TrackNumber ||
			indexColumn == ColumnIndex::Track::Bitrate ||
			indexColumn == ColumnIndex::Track::Length ||
			indexColumn == ColumnIndex::Track::Filesize ||
			indexColumn == ColumnIndex::Track::Discnumber)
		{
			alignment |= Qt::AlignRight;
		}

		else if(indexColumn == ColumnIndex::Track::Filetype ||
				indexColumn == ColumnIndex::Track::Year ||
				indexColumn == ColumnIndex::Track::ModifiedDate ||
				indexColumn == ColumnIndex::Track::AddedDate)
		{
			alignment |= Qt::AlignCenter;
		}

		else
		{
			alignment |= Qt::AlignLeft;
		}

		return alignment;
	}

	else if (role == Qt::DisplayRole || role==Qt::EditRole)
	{
		const MetaData& md = tracks[row];

		switch(indexColumn)
		{
			case ColumnIndex::Track::TrackNumber:
				return QVariant( md.trackNumber() );

			case ColumnIndex::Track::Title:
				return QVariant( md.title() );

			case ColumnIndex::Track::Artist:
				return QVariant( md.artist() );

			case ColumnIndex::Track::Length:
				return QVariant(::Util::msToString(md.durationMs(), "$He $M:$S"));

			case ColumnIndex::Track::Album:
				return QVariant(md.album());

			case ColumnIndex::Track::Discnumber:
				return QVariant(QString::number(md.discnumber()));

			case ColumnIndex::Track::Year:
				if(md.year() == 0){
					return Lang::get(Lang::UnknownYear);
				}

				return md.year();

			case ColumnIndex::Track::Bitrate:
			{
				auto br = (md.bitrate() / 1000);
				return (br == 0) ? "-" : QString::number(br) + " " + tr("kBit/s");
			}

			case ColumnIndex::Track::Filesize:
				return ::Util::File::getFilesizeString(md.filesize());

			case ColumnIndex::Track::Filetype:
			{
				const QString ext = ::Util::File::getFileExtension(md.filepath());
				return (ext.isEmpty()) ? "-" : ext;
			}

			case ColumnIndex::Track::AddedDate:
			{
				const QString format = m->locale.dateFormat(QLocale::ShortFormat);
				return md.createdDateTime().date().toString(format);
			}

			case ColumnIndex::Track::ModifiedDate:
			{
				const QString format = m->locale.dateFormat(QLocale::ShortFormat);
				return md.modifiedDateTime().date().toString(format);
			}

			case ColumnIndex::Track::Rating:
			{
				if(role == Qt::DisplayRole) {
					return QVariant();
				}

				Rating rating = md.rating();
				if(row == m->tmpRating.first)
				{
					rating = m->tmpRating.second;
				}

				return QVariant::fromValue(rating);
			}

			default:
				return QVariant();
		}
	}

	else if(role == Qt::SizeHintRole){
		return QSize(1, Gui::Util::viewRowHeight());
	}

	return QVariant();
}

Qt::ItemFlags TrackModel::flags(const QModelIndex& index = QModelIndex()) const
{
	if (!index.isValid()) {
		return Qt::ItemIsEnabled;
	}

	auto columnIndex = ColumnIndex::Track(index.column());
	if(columnIndex == ColumnIndex::Track::Rating) {
		return (QAbstractTableModel::flags(index) | Qt::ItemIsEditable);
	}

	return QAbstractTableModel::flags(index);
}

bool TrackModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if((index.column() != int(ColumnIndex::Track::Rating) ||
	   (role != Qt::EditRole)))
	{
		return false;
	}

	int row = index.row();

	const MetaDataList& tracks = library()->tracks();
	if(row >= 0 && row < tracks.count())
	{
		const MetaData& md = tracks[row];
		Rating rating = value.value<Rating>();

		if(md.rating() != rating)
		{
			m->tmpRating.first = row;
			m->tmpRating.second = rating;

			if(!m->uto)
			{
				m->uto = new Tagging::UserOperations(-1, this);
			}

			emit dataChanged
			(
				this->index(row, int(ColumnIndex::Track::Rating)),
				this->index(row, int(ColumnIndex::Track::Rating))
			);

			m->uto->setTrackRating(md, rating);

			return true;
		}
	}

	return false;
}

void TrackModel::trackChanged(int row)
{
	m->tmpRating.first = -1;

	emit dataChanged
	(
		this->index(row, 0), this->index(row, columnCount())
	);
}

int TrackModel::rowCount(const QModelIndex&) const
{
	const AbstractLibrary* l = library();
	const MetaDataList& v_md = l->tracks();

	return v_md.count();
}

Id TrackModel::mapIndexToId(int row) const
{
	const MetaDataList& tracks = library()->tracks();

	if(!Util::between(row, tracks)){
		return -1;
	}

	else {
		return tracks[row].id();
	}
}

QString TrackModel::searchableString(int row) const
{
	const MetaDataList& tracks = library()->tracks();

	if(!Util::between(row, tracks)){
		return QString();
	}

	else {
		return tracks[row].title();
	}
}

Cover::Location TrackModel::cover(const IndexSet& indexes) const
{
	if(indexes.isEmpty()){
		return Cover::Location();
	}

	const MetaDataList& tracks = library()->tracks();
	Util::Set<AlbumId> albumIds;

	for(int idx : indexes)
	{
		if(!Util::between(idx, tracks)){
			continue;
		}

		albumIds.insert( tracks[idx].albumId() );
		if(albumIds.size() > 1) {
			return Cover::Location();
		}
	}

	return Cover::Location::coverLocation( tracks.first() );
}

int TrackModel::searchableColumn() const
{
	return int(ColumnIndex::Track::Title);
}

const MetaDataList& Library::TrackModel::selectedMetadata() const
{
	return library()->currentTracks();
}

void TrackModel::languageChanged()
{
	m->locale = Util::Language::getCurrentLocale();
}
