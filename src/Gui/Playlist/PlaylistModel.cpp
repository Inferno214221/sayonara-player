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
#include "Components/Tagging/UserTaggingOperations.h"
#include "Components/Covers/CoverLocation.h"

#include "Gui/Utils/CustomMimeData.h"
#include "Gui/Utils/GuiUtils.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Set.h"
#include "Utils/globals.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Library/SearchMode.h"

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
	QHash<AlbumId, QPixmap>	pms;
	int						oldRowCount;
	int						dragIndex;
	int						rowHeight;
	PlaylistPtr				pl=nullptr;
	Tagging::UserOperations* uto=nullptr;

	Private(PlaylistPtr pl) :
		oldRowCount(0),
		dragIndex(-1),
		rowHeight(20),
		pl(pl)
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
		int point_size = GetSetting(Set::PL_FontSize);
		if(point_size > 0){
			f.setPointSize(point_size);
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

			if(!m->pms.contains(md.albumId()))
			{
				int height = m->rowHeight - 6;

				Cover::Location cl = Cover::Location::coverLocation(md);
				m->pms[md.albumId()] = QPixmap(cl.preferredPath()).scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			}

			return m->pms[md.albumId()];
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
	MetaDataList v_md;
	v_md.reserve(indexes.size());

	for(auto idx : indexes)
	{
		MetaData md = m->pl->track(idx);
		if(rating != md.rating())
		{
			v_md << md;
			md.setRating(rating);

			m->pl->replaceTrack(idx, md);
		}

		emit dataChanged(index(idx, 0), index(idx, int(ColumnName::Description)));
	}

	if(v_md.isEmpty()){
		return;
	}

	if(!m->uto)
	{
		m->uto = new Tagging::UserOperations(-1, this);
	}

	m->uto->setTrackRating(v_md, rating);
}

void Model::insertTracks(const MetaDataList& v_md, int row)
{
	auto* plh = Handler::instance();
	plh->insertTracks(v_md, row, m->pl->index());
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
	MetaDataList v_md;
	v_md.reserve(rows.size());

	for(int row : rows)
	{
		v_md << m->pl->track(row);
	}

	return v_md;
}

QModelIndexList Model::searchResults(const QString& substr)
{
	QModelIndexList ret;
	QString pure_search_string = substr;
	PlaylistSearchMode plsm = PlaylistSearchMode::Title;

	if(pure_search_string.startsWith(ARTIST_SEARCH_PREFIX))
	{
		plsm = PlaylistSearchMode::Artist;
		pure_search_string.remove(ARTIST_SEARCH_PREFIX);
	}
	else if(pure_search_string.startsWith(ALBUM_SEARCH_PREFIX))
	{
		plsm = PlaylistSearchMode::Album;
		pure_search_string.remove(ALBUM_SEARCH_PREFIX);
	}
	else if(pure_search_string.startsWith(JUMP_PREFIX))
	{
		plsm = PlaylistSearchMode::Jump;
		pure_search_string.remove(JUMP_PREFIX);
	}

	pure_search_string = pure_search_string.trimmed();

	if(plsm == PlaylistSearchMode::Jump)
	{
		bool ok;
		int line = pure_search_string.toInt(&ok);
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
		if(str.contains(pure_search_string))
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

	QList<int> rows;
	for(const QModelIndex& index : indexes)
	{
		if(!rows.contains(index.row()))
		{
			rows << index.row();
		}
	}

	Algorithm::sort(rows, [](int row1, int row2){
		return (row1 < row2);
	});

	MetaDataList v_md;
	v_md.reserve( size_t(rows.size()) );

	for(int row : Algorithm::AsConst(rows))
	{
		if(row >= m->pl->count()){
			continue;
		}

		v_md << m->pl->track(row);
	}

	if(v_md.empty()){
		return nullptr;
	}

	auto* mimedata = new Gui::CustomMimeData(this);
	mimedata->setMetadata(v_md);
	mimedata->setPlaylistSourceIndex(m->pl->index());

	return mimedata;
}

bool Model::hasLocalMedia(const IndexSet& rows) const
{
	const  MetaDataList& tracks = m->pl->tracks();

	for(int row : rows)
	{
		if(!Util::File::isWWW(tracks[row].filepath())){
			return true;
		}
	}

	return false;
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

void Model::setRowHeight(int row_height)
{
	if(m->rowHeight != row_height)
	{
		m->pms.clear();
		m->rowHeight = row_height;
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

