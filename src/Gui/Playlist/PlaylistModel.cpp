/* PlaylistItemModel.cpp */

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
* PlaylistItemModel.cpp
 *
 *  Created on: Apr 8, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "PlaylistModel.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/ExternTracksPlaylistGenerator.h"
#include "Components/Tagging/UserTaggingOperations.h"
#include "Components/Covers/CoverLocation.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Gui/Utils/GuiUtils.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Set.h"
#include "Utils/globals.h"

#include "Utils/Language/Language.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Tagging/TaggingCover.h"

#include <QUrl>
#include <QPalette>
#include <QHash>
#include <QPixmap>
#include <QMainWindow>

namespace Algorithm=Util::Algorithm;
using Playlist::Model;
using Playlist::Handler;

static const QChar ALBUM_SEARCH_PREFIX='%';
static const QChar ARTIST_SEARCH_PREFIX='$';
static const QChar JUMP_PREFIX=':';

enum class PlaylistSearchMode
{
	Artist,
	Album,
	Title,
	Jump
};

struct Model::Private
{
	QHash<AlbumId, QPixmap>	coverLookupMap;
	int						oldRowCount;
	int						dragIndex;
	int						rowHeight;
	PlaylistPtr				pl;
	Tagging::UserOperations* uto=nullptr;
	Handler* playlistHandler;

	Private(PlaylistPtr pl) :
		oldRowCount(0),
		dragIndex(-1),
		rowHeight(20),
		pl(pl),
		playlistHandler{::Playlist::HandlerProvider::instance()->handler()}
	{}
};

Model::Model(PlaylistPtr pl, QObject* parent) :
	SearchableTableModel(parent)
{
	m = Pimpl::make<Private>(pl);

	connect(m->pl.get(), &Playlist::Playlist::sigItemsChanged, this, &Model::playlistChanged);

	ListenSettingNoCall(Set::PL_EntryLook, Model::lookChanged);

	playlistChanged(0);
}

Model::~Model() = default;

int Model::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return m->pl->count();
}

int Model::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return int(ColumnName::NumColumns);
}

static QString convertEntryLook(const QString& entryLook, const MetaData& md)
{
	QString ret(entryLook);
	ret.replace("*", QChar(Model::Bold));
	ret.replace("'", QChar(Model::Italic));

	ret.replace("%title%", md.title());
	ret.replace("%nr%", QString::number(md.trackNumber()));
	ret.replace("%artist%", md.artist());
	ret.replace("%album%", md.album());

	return ret;
}

QVariant Model::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	int col = index.column();

	if ( !Util::between(row, m->pl->count())) {
		return QVariant();
	}

	if (role == Qt::DisplayRole)
	{
		if(col == ColumnName::TrackNumber) {
			return QString("%1.").arg(row + 1);
		}

		else if(col == ColumnName::Time) {
			auto l = m->pl->track(row).durationMs();
			if(l / 1000 <= 0){
				return QVariant();
			}

			return Util::msToString(l, "$M:$S");
		}

		return QVariant();
	}

	else if(role == Qt::EditRole)
	{
		if(col == ColumnName::Description )
		{
			MetaData md = m->pl->track(row);
			Rating rating = metadata(row).rating();

			if(md.radioMode() != RadioMode::Off)
			{
				rating = Rating::Last;
			}

			return QVariant::fromValue(rating);
		}
	}

	else if (role == Qt::TextAlignmentRole)
	{
		if( col != ColumnName::Description){
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		}
	}

	else if (role == Qt::BackgroundRole)
	{
		if(m->pl->currentTrackIndex() == row)
		{
			QPalette palette = Gui::Util::mainWindow()->palette();
			QColor col_highlight = palette.color(QPalette::Active, QPalette::Highlight);
			col_highlight.setAlpha(80);
			return col_highlight;
		}
	}

	else if(role == Qt::FontRole)
	{
		QFont f = Gui::Util::mainWindow()->font();
		int pointSize = GetSetting(Set::PL_FontSize);
		if(pointSize > 0){
			f.setPointSize(pointSize);
		}

		if(col == ColumnName::TrackNumber)
		{
			f.setBold(true);
		}

		return f;
	}

	else if(role == Qt::DecorationRole)
	{
		if(col == ColumnName::Cover)
		{
			MetaData md = m->pl->track(row);

			if(!m->coverLookupMap.contains(md.albumId()))
			{
				int height = m->rowHeight - 6;

				Cover::Location cl = Cover::Location::coverLocation(md);
				DB::Covers* coverDb = DB::Connector::instance()->coverConnector();

				QPixmap cover;
				const QString hash = cl.hash();
				coverDb->getCover(hash, cover);

				if(cover.isNull()) {
					cover = QPixmap(cl.preferredPath());
				}

				if(!cover.isNull()) {
					m->coverLookupMap[md.albumId()] = cover.scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				}
			}

			return m->coverLookupMap[md.albumId()];
		}
	}

	else if(role == Qt::SizeHintRole)
	{
		if(col == ColumnName::Cover)
		{
			int h = m->rowHeight - 4;
			return QSize(h, h);
		}
	}

	else if(role == Model::EntryLookRole)
	{
		if(col == ColumnName::Description)
		{
			return convertEntryLook(GetSetting(Set::PL_EntryLook), m->pl->track(row));
		}
	}

	else if(role == Model::DragIndexRole)
	{
		return (row == m->dragIndex);
	}

	else if(role == Model::RatingRole)
	{
		return QVariant::fromValue<Rating>(m->pl->track(row).rating());
	}

	else if(role == Model::RadioModeRole)
	{
		return int(m->pl->track(row).radioMode());
	}

	return QVariant();
}


bool Model::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(role == Qt::EditRole && index.isValid())
	{
		int row = index.row();
		changeRating({row}, value.value<Rating>());
		return true;
	}

	return false;
}


Qt::ItemFlags Model::flags(const QModelIndex& index) const
{
	int row = index.row();

	if(!index.isValid()){
		return (Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
	}

	if(Util::between(row, m->pl->count()))
	{
		const MetaData& md = metadata(row);
		if(md.isDisabled()){
			return Qt::NoItemFlags;
		}
	}

	Qt::ItemFlags item_flags = QAbstractTableModel::flags(index);
	if(index.column() == int(ColumnName::Description))
	{
		item_flags |= Qt::ItemIsEditable;
	}

	return (item_flags | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
}

void Model::clear()
{
	m->pl->clear();
}

void Model::removeTracks(const IndexSet& indexes)
{
	m->pl->removeTracks(indexes);
}

IndexSet Model::moveTracks(const IndexSet& indexes, int target_index)
{
	return m->pl->moveTracks(indexes, target_index);
}

IndexSet Model::moveTracksUp(const IndexSet& indexes)
{
	int min_row = *(std::min_element(indexes.begin(), indexes.end()));
	if(min_row <= 0){
		return IndexSet();
	}

	return moveTracks(indexes, min_row - 1);
}

IndexSet Model::moveTracksDown(const IndexSet& indexes)
{
	auto min_max = std::minmax_element(indexes.begin(), indexes.end());
	int min_row = *(min_max.first);
	int max_row = *(min_max.second);

	if(max_row >= rowCount() - 1){
		return IndexSet();
	}

	return moveTracks(indexes, min_row + int(indexes.size()) + 1);
}

IndexSet Model::copyTracks(const IndexSet& indexes, int target_index)
{
	return m->pl->copyTracks(indexes, target_index);
}

void Model::changeRating(const IndexSet& indexes, Rating rating)
{
	MetaDataList tracks;
	tracks.reserve(indexes.size());

	for(auto idx : indexes)
	{
		MetaData md = m->pl->track(idx);
		if(rating != md.rating())
		{
			tracks << md;
			md.setRating(rating);

			m->pl->replaceTrack(idx, md);
		}

		emit dataChanged(index(idx, 0), index(idx, int(ColumnName::Description)));
	}

	if(tracks.isEmpty()){
		return;
	}

	if(!m->uto)
	{
		m->uto = new Tagging::UserOperations(-1, this);
	}

	m->uto->setTrackRating(tracks, rating);
}

void Model::insertTracks(const MetaDataList& tracks, int row)
{
	m->pl->insertTracks(tracks, row);
}

void Model::insertTracks(const QStringList& files, int row)
{
	auto* playlistGenerator = new ExternTracksPlaylistGenerator(m->playlistHandler, m->pl);
	connect(playlistGenerator, &ExternTracksPlaylistGenerator::sigFinished, playlistGenerator, &QObject::deleteLater);
	playlistGenerator->insertPaths(files, row);
}

int Model::currentTrack() const
{
	return m->pl->currentTrackIndex();
}

void Model::setCurrentTrack(int row)
{
	m->pl->changeTrack(row);
}

MetaData Model::metadata(int row) const
{
	return m->pl->track(row);
}

MetaDataList Model::metadata(const IndexSet &rows) const
{
	MetaDataList tracks;
	tracks.reserve(rows.size());

	Util::Algorithm::transform(rows, tracks, [this](int row)
	{
		return m->pl->track(row);
	});

	return tracks;
}

QModelIndexList Model::searchResults(const QString& substr)
{
	QModelIndexList ret;
	QString pureSearchString = substr;
	PlaylistSearchMode plsm = PlaylistSearchMode::Title;

	if(pureSearchString.startsWith(ARTIST_SEARCH_PREFIX))
	{
		plsm = PlaylistSearchMode::Artist;
		pureSearchString.remove(ARTIST_SEARCH_PREFIX);
	}
	else if(pureSearchString.startsWith(ALBUM_SEARCH_PREFIX))
	{
		plsm = PlaylistSearchMode::Album;
		pureSearchString.remove(ALBUM_SEARCH_PREFIX);
	}
	else if(pureSearchString.startsWith(JUMP_PREFIX))
	{
		plsm = PlaylistSearchMode::Jump;
		pureSearchString.remove(JUMP_PREFIX);
	}

	pureSearchString = pureSearchString.trimmed();

	if(plsm == PlaylistSearchMode::Jump)
	{
		bool ok;
		int line = pureSearchString.toInt(&ok);
		if(ok && line < rowCount()) {
			ret << this->index(line, 0);
		}

		else {
			ret << QModelIndex();
		}

		return ret;
	}

	int rows = rowCount();
	for(int i=0; i<rows; i++)
	{
		MetaData md = m->pl->track(i);
		QString str;
		switch(plsm)
		{
			case PlaylistSearchMode::Artist:
				str = md.artist();
				break;
			case PlaylistSearchMode::Album:
				str = md.album();
				break;
			default:
				str = md.title();
				break;
		}

		str = Library::Utils::convertSearchstring(str, searchMode());
		if(str.contains(pureSearchString))
		{
			ret << this->index(i, 0);
		}
	}

	return ret;
}
using ExtraTriggerMap=SearchableModelInterface::ExtraTriggerMap;
ExtraTriggerMap Model::getExtraTriggers()
{
	ExtraTriggerMap map;

	map.insert(ARTIST_SEARCH_PREFIX, Lang::get(Lang::Artist));
	map.insert(ALBUM_SEARCH_PREFIX, Lang::get(Lang::Album));
	map.insert(JUMP_PREFIX, tr("Goto row"));

	return map;
}

QMimeData* Model::mimeData(const QModelIndexList& indexes) const
{
	if(indexes.isEmpty()){
		return nullptr;
	}

	Util::Set<int> rowSet;
	for(const QModelIndex& index : indexes)
	{
		rowSet << index.row();
	}

	QList<int> rows = rowSet.toList();
	Algorithm::sort(rows, [](int row1, int row2){
		return (row1 < row2);
	});

	MetaDataList tracks;
	tracks.reserve( size_t(rows.size()) );

	for(int row : Algorithm::AsConst(rows))
	{
		if(row >= m->pl->count()){
			continue;
		}

		tracks << m->pl->track(row);
	}

	if(tracks.empty()){
		return nullptr;
	}

	auto* mimedata = new Gui::CustomMimeData(this);
	mimedata->setMetadata(tracks);
	mimedata->setPlaylistSourceIndex(m->pl->index());

	return mimedata;
}

bool Model::hasLocalMedia(const IndexSet& rows) const
{
	const  MetaDataList& tracks = m->pl->tracks();

	return Algorithm::contains(rows, [tracks](int row)
	{
		return (!Util::File::isWWW(tracks[row].filepath()));
	});
}

void Model::setDragIndex(int dragIndex)
{
	if(Util::between(m->dragIndex, rowCount()))
	{
		emit dataChanged(index(m->dragIndex, 0), index(m->dragIndex, columnCount() - 1));
	}

	m->dragIndex = dragIndex;

	if(Util::between(dragIndex, rowCount()))
	{
		emit dataChanged(index(dragIndex, 0), index(dragIndex, columnCount() - 1));
	}
}

void Model::setRowHeight(int rowHeight)
{
	if(m->rowHeight != rowHeight)
	{
		m->coverLookupMap.clear();
		m->rowHeight = rowHeight;
	}
}

void Model::refreshData()
{
	m->pl->enableAll();
}

void Model::lookChanged()
{
	emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
}

void Model::playlistChanged(int pl_idx)
{
	Q_UNUSED(pl_idx)

	if(m->oldRowCount > m->pl->count())
	{
		beginRemoveRows(QModelIndex(), m->pl->count(), m->oldRowCount - 1);
		endRemoveRows();
	}

	else if(m->pl->count() > m->oldRowCount){
		beginInsertRows(QModelIndex(), m->oldRowCount, m->pl->count() - 1);
		endInsertRows();
	}

	if(m->pl->count() == 0)
	{
		beginResetModel();
		endResetModel();
	}

	m->oldRowCount = m->pl->count();

	emit dataChanged(index(0,0), index(rowCount()-1, columnCount()-1));
	emit sigDataReady();
}
