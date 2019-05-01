/* LibraryItemModelTracks.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
 *      Author: Lucio Carreras
 */

#include "TrackModel.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Tagging/UserTaggingOperations.h"

#include "GUI/Library/Utils/ColumnIndex.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language.h"
#include "Utils/Set.h"

using namespace Library;

TrackModel::TrackModel(QObject* parent, AbstractLibrary* library) :
	ItemModel(parent, library)
{}

TrackModel::~TrackModel() {}

QVariant TrackModel::data(const QModelIndex &index, int role) const
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

	ColumnIndex::Track idx_col = (ColumnIndex::Track) col;

	if (role == Qt::TextAlignmentRole)
	{
		int alignment = Qt::AlignVCenter;

		if (idx_col == ColumnIndex::Track::TrackNumber ||
			idx_col == ColumnIndex::Track::Bitrate ||
			idx_col == ColumnIndex::Track::Length ||
			idx_col == ColumnIndex::Track::Year ||
			idx_col == ColumnIndex::Track::Filesize ||
			idx_col == ColumnIndex::Track::Discnumber)
		{
			alignment |= Qt::AlignRight;
		}

		else {
			alignment |= Qt::AlignLeft;
		}

		return alignment;
	}

	else if (role == Qt::DisplayRole || role==Qt::EditRole)
	{
		const MetaData& md = tracks[row];

		switch(idx_col)
		{
			case ColumnIndex::Track::TrackNumber:
				return QVariant( md.track_num );

			case ColumnIndex::Track::Title:
				return QVariant( md.title() );

			case ColumnIndex::Track::Artist:
				return QVariant( md.artist() );

			case ColumnIndex::Track::Length:
				return QVariant( ::Util::cvt_ms_to_string(md.length_ms) );

			case ColumnIndex::Track::Album:
				return QVariant(md.album());

			case ColumnIndex::Track::Discnumber:
				return QVariant(Lang::get(Lang::Disc) + " " + QString::number(md.discnumber));

			case ColumnIndex::Track::Year:
				if(md.year == 0){
					return Lang::get(Lang::None);
				}

				return md.year;

			case ColumnIndex::Track::Bitrate:
				return QString::number(md.bitrate / 1000) + " kbit/s";

			case ColumnIndex::Track::Filesize:
				return ::Util::File::calc_filesize_str(md.filesize);

			case ColumnIndex::Track::Rating:
				if(role == Qt::DisplayRole) {
					return QVariant();
				}

				return QVariant(md.rating);

			default:
				return QVariant();
		}
	}

	return QVariant();
}


Qt::ItemFlags TrackModel::flags(const QModelIndex &index = QModelIndex()) const
{
	if (!index.isValid()) {
		return Qt::ItemIsEnabled;
	}

	if(index.column() == (int) ColumnIndex::Track::Rating) {
		return (QAbstractTableModel::flags(index) | Qt::ItemIsEditable);
	}

	return QAbstractTableModel::flags(index);
}

bool TrackModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(!index.isValid()){
		return false;
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		int row = index.row();
		int col = index.column();

		if(col == (int) ColumnIndex::Track::Rating)
		{
			MetaData md = library()->tracks()[row];

			Tagging::UserOperations* uto = new Tagging::UserOperations(-1, this);
			connect(uto, &Tagging::UserOperations::sig_finished, uto, &Tagging::UserOperations::deleteLater);
			uto->set_track_rating(md, (Rating) (value.toInt()));

			return true;
		}
	}

	return false;
}

int TrackModel::rowCount(const QModelIndex&) const
{
	const AbstractLibrary* l = library();
	const MetaDataList& v_md = l->tracks();

	return v_md.count();
}


Id TrackModel::id_by_index(int row) const
{
	const MetaDataList& tracks = library()->tracks();

	if(!between(row, tracks)){
		return -1;
	}

	else {
		return tracks[row].id;
	}
}

QString TrackModel::searchable_string(int row) const
{
	const MetaDataList& tracks = library()->tracks();

	if(!between(row, tracks)){
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
	Util::Set<AlbumId> album_ids;

	for(int idx : indexes)
	{
		if(!between(idx, tracks)){
			continue;
		}

		album_ids.insert( tracks[idx].album_id );
		if(album_ids.size() > 1) {
			return Cover::Location();
		}
	}

	return Cover::Location::cover_location( tracks.first() );
}


int TrackModel::searchable_column() const
{
	return (int) ColumnIndex::Track::Title;
}


const Util::Set<Id>& TrackModel::selections() const
{
	return library()->selected_tracks();
}


const MetaDataList& Library::TrackModel::mimedata_tracks() const
{
	return library()->current_tracks();
}
