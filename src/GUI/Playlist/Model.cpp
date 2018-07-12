/* PlaylistItemModel.cpp */

/* Copyright (C) 2011-2017 Lucio Carreras
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
 *      Author: Lucio Carreras
 */

#include "Model.h"
#include "Components/Playlist/AbstractPlaylist.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Tagging/Editor.h"
#include "Components/Covers/CoverLocation.h"

#include "GUI/Utils/CustomMimeData.h"
#include "GUI/Utils/GuiUtils.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Set.h"
#include "Utils/globals.h"
#include "Utils/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include <QUrl>
#include <QPalette>
#include <QHash>
#include <QPixmap>
#include <QMainWindow>

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

struct PlaylistItemModel::Private
{
	QHash<AlbumId, QPixmap>	pms;
	int						old_row_count;
	PlaylistPtr				pl=nullptr;

	Private(PlaylistPtr pl) :
		old_row_count(0),
		pl(pl)
	{}
};

PlaylistItemModel::PlaylistItemModel(PlaylistPtr pl, QObject* parent) :
	SearchableTableModel(parent)
{
	m = Pimpl::make<Private>(pl);

	connect(m->pl.get(), &Playlist::Base::sig_items_changed, this, &PlaylistItemModel::playlist_changed);

	playlist_changed(0);
}

PlaylistItemModel::~PlaylistItemModel() {}

int PlaylistItemModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return m->pl->count();
}

int PlaylistItemModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return (int)(ColumnName::NumColumns);
}


QVariant PlaylistItemModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	int col = index.column();

	if ( !between(row, m->pl->count())) {
		return QVariant();
	}

	if (role == Qt::DisplayRole	|| role==Qt::EditRole)
	{
		if(col ==  ColumnName::TrackNumber) {
			return QString("%1.").arg(row + 1);
		}

		else if(col == ColumnName::Time) {
			auto l = m->pl->metadata(row).length_ms;
			return Util::cvt_ms_to_string(l, true, true, false);
		}

		return QVariant();
	}

	else if (role == Qt::TextAlignmentRole)
	{
		if( col != ColumnName::Description){
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		}
	}

	else if (role == Qt::BackgroundColorRole)
	{
		if(m->pl->current_track_index() == row)
		{
			QPalette palette = Gui::Util::main_window()->palette();
			QColor col_highlight = palette.color(QPalette::Active, QPalette::Highlight);
			col_highlight.setAlpha(80);
			return col_highlight;
		}
	}

	else if(role == Qt::FontRole)
	{
		Settings* s = Settings::instance();
		QFont f = Gui::Util::main_window()->font();
		int point_size = s->get<Set::PL_FontSize>();
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
			AlbumId album_id = m->pl->metadata(row).album_id;
			if(!m->pms.contains(album_id))
			{
				Cover::Location cl = Cover::Location::cover_location(m->pl->metadata(row));
				m->pms[album_id] = QPixmap(cl.preferred_path()).scaled(QSize(20, 20), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			}

			return m->pms[album_id];
		}
	}

	return QVariant();
}

bool PlaylistItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(role == Qt::EditRole && index.isValid())
	{
		int row = index.row();
		change_rating({row}, (Rating) (value.toInt()) );
		return true;
	}

	return false;
}


Qt::ItemFlags PlaylistItemModel::flags(const QModelIndex &index) const
{
	int row = index.row();
	if (!index.isValid()){
		return (Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
	}

	if( between(row, m->pl->count()))
	{
		const MetaData& md = metadata(row);
		if(md.is_disabled){
			return Qt::NoItemFlags;
		}
	}

	Qt::ItemFlags item_flags = QAbstractTableModel::flags(index);
	if(index.column() == ColumnName::Description)
	{
		item_flags |= Qt::ItemIsEditable;
	}

	return (item_flags | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
}

void PlaylistItemModel::clear()
{
	m->pl->clear();
}

void PlaylistItemModel::remove_rows(const IndexSet& indexes)
{
	m->pl->remove_tracks(indexes);
}

IndexSet PlaylistItemModel::move_rows(const IndexSet& indexes, int target_index)
{
	return m->pl->move_tracks(indexes, target_index);
}

IndexSet PlaylistItemModel::move_rows_up(const IndexSet& indexes)
{
	int min_row = *(std::min_element(indexes.begin(), indexes.end()));
	if(min_row <= 0){
		return IndexSet();
	}

	return move_rows(indexes, min_row - 1);
}


IndexSet PlaylistItemModel::move_rows_down(const IndexSet& indexes)
{
	auto min_max = std::minmax_element(indexes.begin(), indexes.end());
	int min_row = *(min_max.first);
	int max_row = *(min_max.second);

	if(max_row >= rowCount() - 1){
		return IndexSet();
	}

	return move_rows(indexes, min_row + indexes.size() + 1);
}

IndexSet PlaylistItemModel::copy_rows(const IndexSet& indexes, int target_index)
{
	return m->pl->copy_tracks(indexes, target_index);
}

void PlaylistItemModel::change_rating(const IndexSet& indexes, Rating rating)
{
	MetaDataList v_md;
	v_md.reserve(indexes.size());
	for(auto idx : indexes)
	{
		v_md << m->pl->metadata(idx);
	}

	Tagging::Editor* te = new Tagging::Editor(v_md);

	for(int i=0; i<v_md.count(); i++)
	{
		MetaData md	= v_md[i];
		md.rating = rating;
		te->update_track(i, md);
	}

	te->commit();

	connect(te, &QThread::finished, te, &Tagging::Editor::deleteLater);
}

void PlaylistItemModel::insert_tracks(const MetaDataList& v_md, int row)
{
	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->insert_tracks(v_md, row, m->pl->index());
}

int PlaylistItemModel::current_track() const
{
	return m->pl->current_track_index();
}

void PlaylistItemModel::set_current_track(int row)
{
	m->pl->change_track(row);
}

const MetaData& PlaylistItemModel::metadata(int row) const
{
	return m->pl->metadata(row);
}

MetaDataList PlaylistItemModel::metadata(const IndexSet &rows) const
{
	MetaDataList v_md;
	v_md.reserve(rows.size());

	for(int row : rows)
	{
		v_md << m->pl->metadata(row);
	}

	return v_md;
}

QModelIndex PlaylistItemModel::getPrevRowIndexOf(const QString& substr, int row, const QModelIndex& parent)
{
	Q_UNUSED(parent)
	return getRowIndexOf(substr, row, false);
}

QModelIndex PlaylistItemModel::getNextRowIndexOf(const QString& substr, int row, const QModelIndex &parent)
{
	Q_UNUSED(parent)
	return getRowIndexOf(substr, row, true);
}

QModelIndex PlaylistItemModel::getRowIndexOf(const QString& substr, int row, bool is_forward)
{
	row = std::min(rowCount()-1, row);

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
			return this->index(line, 0);
		}

		return index(-1, -1);
	}

	for(int i=0; i<rowCount(); i++)
	{
		int row_idx;
		if(is_forward)
		{
			row_idx = (i + row) % rowCount();
		}

		else
		{
			if(row - i < 0) {
				row = rowCount() - 1;
			}

			row_idx = (row - i) % rowCount();
		}

		MetaData md = m->pl->metadata(row_idx);
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

		str = Library::Util::convert_search_string(str, search_mode());
		if(str.contains(pure_search_string))
		{
			return this->index(row_idx, 0);
		}
	}

	return index(-1, -1);
}

using ExtraTriggerMap=SearchableModelInterface::ExtraTriggerMap;
ExtraTriggerMap PlaylistItemModel::getExtraTriggers()
{
	ExtraTriggerMap map;

	map.insert(ARTIST_SEARCH_PREFIX, Lang::get(Lang::Artist));
	map.insert(ALBUM_SEARCH_PREFIX, Lang::get(Lang::Album));
	map.insert(JUMP_PREFIX, tr("Goto row"));

	return map;
}


QMimeData* PlaylistItemModel::mimeData(const QModelIndexList& indexes) const
{
	if(indexes.isEmpty()){
		return nullptr;
	}

	CustomMimeData* mimedata = new CustomMimeData(this);

	MetaDataList v_md;
	v_md.reserve(indexes.size());

	SP::Set<int> rows;
	for(auto idx : indexes) {
		rows.insert(idx.row());
	}

	for(int row : rows)
	{
		if(row >= m->pl->count()){
			continue;
		}

		v_md << m->pl->metadata(row);
	}

	if(v_md.empty()){
		return nullptr;
	}

	mimedata->set_metadata(v_md);
	mimedata->set_playlist_source_index(m->pl->index());

	return mimedata;
}

bool PlaylistItemModel::has_local_media(const IndexSet& rows) const
{
	const  MetaDataList& tracks = m->pl->playlist();

	for(int row : rows)
	{
		if(!Util::File::is_www(tracks[row].filepath())){
			return true;
		}
	}

	return false;
}

void PlaylistItemModel::playlist_changed(int pl_idx)
{
	Q_UNUSED(pl_idx)

	if(m->old_row_count > m->pl->count())
	{
		beginRemoveRows(QModelIndex(), m->pl->count(), m->old_row_count - 1);
		endRemoveRows();
	}

	else if(m->pl->count() > m->old_row_count){
		beginInsertRows(QModelIndex(), m->old_row_count, m->pl->count() - 1);
		endInsertRows();
	}

	if(m->pl->is_empty()){
		beginResetModel();
		endResetModel();
	}

	m->old_row_count = m->pl->count();

	emit dataChanged(index(0,0), index(rowCount()-1, columnCount()-1));
	emit sig_data_ready();
}

