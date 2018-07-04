/* AlbumCoverModel.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "CoverModel.h"
#include "AlbumCoverFetchThread.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookup.h"

#include "Utils/MetaData/Album.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>
#include <QPixmap>
#include <QThread>

using Cover::Location;
using Cover::Lookup;
using Library::CoverModel;

struct CoverModel::Private
{
	AlbumCoverFetchThread*		cover_thread=nullptr;
	QHash<QString, QPixmap>		pixmaps;
	QHash<QString, Location>	cover_locations;
	QHash<QString, QModelIndex> indexes;
	int							old_row_count;
	int							old_column_count;

	int zoom;
	int columns;
	int n_threads_running;

	Private(QObject* parent)
	{
		cover_thread = new AlbumCoverFetchThread(parent);

		old_row_count = 0;
		old_column_count = 0;
		zoom = Settings::instance()->get<Set::Lib_CoverZoom>();
		columns = 10;
		n_threads_running = 0;
	}

	~Private()
	{
		cover_thread->stop();
		while(cover_thread->isRunning()){
			::Util::sleep_ms(50);
		}
	}

	QSize item_size() const
	{
		return QSize(zoom + 50, zoom + 50);
	}
};

static QString get_hash(const Album& album)
{
	return album.name() + "-" + album.id;
}

CoverModel::CoverModel(QObject* parent, AbstractLibrary* library) :
	ItemModel(parent, library)
{
	m = Pimpl::make<Private>(this);

	connect(m->cover_thread, &AlbumCoverFetchThread::sig_next, this, &CoverModel::next_hash);
}

CoverModel::~CoverModel()
{
	if(m->cover_thread){
		m->cover_thread->stop();
	}
}

int CoverModel::rowCount(const QModelIndex& parent) const
{
	if(columnCount() == 0){
		return 0;
	}

	Q_UNUSED(parent);
	return (albums().size() / columnCount()) + 1;
}

int CoverModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return m->columns;
}

void CoverModel::refresh_data()
{
	int old_columns = m->old_column_count;
	int old_rows = m->old_row_count;

	int new_rows = rowCount();
	int new_columns = columnCount();

	if(new_columns > old_columns) {
		add_columns(old_columns, new_columns - old_columns);
	}

	else if(new_columns < old_columns) {
		remove_columns(new_columns, old_columns - new_columns);
	}

	if(new_rows > old_rows)	{
		add_rows(old_rows, new_rows - old_rows);
	}

	else if(new_rows < old_rows) {
		remove_rows(new_rows, old_rows - new_rows);
	}

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {Qt::DisplayRole, Qt::SizeHintRole});
}

void CoverModel::add_rows(int row, int count)
{
	beginInsertRows(QModelIndex(), row, row + count - 1);
	m->old_row_count += count;
	endInsertRows();
}

void CoverModel::remove_rows(int row, int count)
{
	beginRemoveRows(QModelIndex(), row, row + count - 1);
	m->old_row_count -= count;
	endRemoveRows();
}

void CoverModel::add_columns(int column, int count)
{
	beginInsertColumns(QModelIndex(), column, column + count - 1);
	m->old_column_count += count;
	endInsertColumns();
}

void CoverModel::remove_columns(int column, int count)
{
	beginRemoveColumns(QModelIndex(), column, column + count - 1);
	m->old_column_count -= count;
	endRemoveColumns();
}


QVariant CoverModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid()) {
		return QVariant();
	}

	int row = index.row();
	int col = index.column();

	int n_columns = columnCount();
	int lin_idx = (row * n_columns) + col;
	if(lin_idx >= albums().count()){
		return QVariant();
	}

	switch(role)
	{
		case Qt::DisplayRole:
			{
				QString name = albums()[lin_idx].name();
				if(name.trimmed().isEmpty()){
					name = Lang::get(Lang::None);
				}

				return name;
			}

		case Qt::TextAlignmentRole:
			return Qt::AlignHCenter;

		case Qt::DecorationRole:
			{
				const Album& album = albums()[lin_idx];
				QString hash = get_hash(album);
				QPixmap p;

				if(!m->pixmaps.contains(hash))
				{
					Location cl;
					if(!m->cover_locations.contains(hash))
					{
						cl = Location::cover_location(album);
						m->cover_locations[hash] = cl;
					}

					else {
						cl = m->cover_locations[hash];
					}

					p = QPixmap(cl.preferred_path());
					if(!Location::is_invalid(cl.preferred_path()))
					{
						m->pixmaps[hash] = p.scaled(m->zoom, m->zoom, Qt::KeepAspectRatio);
					}

					else {
						if(!m->cover_thread->isRunning()){
							m->cover_thread->start();
						}

						m->indexes[hash] = index;
						m->cover_thread->add_data(hash, cl);
					}
				}

				else{
					p = m->pixmaps[hash];
					if(p.size().width() < m->zoom - 20)
					{
						m->pixmaps.remove(hash);
					}
				}

				return p.scaled(m->zoom, m->zoom, Qt::KeepAspectRatio);
			}

		case Qt::SizeHintRole:
			return m->item_size();

		default:
			return QVariant();
	}

	return QVariant();
}


void CoverModel::next_hash()
{
	AlbumCoverFetchThread* acft = dynamic_cast<AlbumCoverFetchThread*>(sender());
	QString hash = acft->current_hash();
	Location cl = acft->current_cover_location();

	QModelIndex idx = m->indexes[hash];

	Lookup* clu = new Lookup(this, 1);
	connect(clu, &Lookup::sig_finished, this, [=](bool success)
	{
		if(success) {
			emit dataChanged(idx, idx);
		}

		if(acft){
			acft->done(success);
		}

		clu->deleteLater();
	});

	clu->fetch_cover(cl);
}

QModelIndex CoverModel::getNextRowIndexOf(const QString& substr, int cur_row, const QModelIndex& parent)
{
	Q_UNUSED(parent)

	for(int i=0; i<albums().count(); i++)
	{
		int idx = (i + cur_row) % albums().size();
		QString title = searchable_string(idx);
		title = Library::Util::convert_search_string(title, search_mode());

		if(title.contains(substr))
		{
			return this->index(idx / columnCount(), idx % columnCount());
		}

		const QStringList artists = albums().at(idx).artists();
		for(const QString& artist : artists)
		{
			QString cvt_artist = Library::Util::convert_search_string(artist, search_mode());

			if(cvt_artist.contains(substr)){
				return this->index(idx / columnCount(), idx % columnCount());
			}
		}
	}

	return QModelIndex();
}

QModelIndex CoverModel::getPrevRowIndexOf(const QString& substr, int row, const QModelIndex& parent)
{
	Q_UNUSED(parent)

	int len = albums().size();
	for(int i=0; i<albums().count(); i++)
	{
		if(row - i < 0){
			row = len - 1;
		}

		int idx = (row - i) % len;

		QString title = searchable_string(idx);
		title = Library::Util::convert_search_string(title, search_mode());
		if(title.contains(substr))
		{
			return this->index(idx / columnCount(), idx % columnCount());
		}

		const QStringList artists = albums().at(idx).artists();
		for(const QString& artist : artists)
		{
			QString cvt_artist = Library::Util::convert_search_string(artist, search_mode());

			if(cvt_artist.contains(substr)){
				return this->index(idx / columnCount(), idx % columnCount());
			}
		}
	}

	return QModelIndex();
}

int CoverModel::getNumberResults(const QString& substr)
{
	int ret=0;
	for(int i=0; i<albums().count(); i++)
	{
		QString title = searchable_string(i);
		title = Library::Util::convert_search_string(title, search_mode());

		if(title.contains(substr))
		{
			ret++;
			continue;
		}

		const QStringList artists = albums().at(i).artists();
		for(const QString& artist : artists)
		{
			QString cvt_artist = Library::Util::convert_search_string(artist, search_mode());

			if(cvt_artist.contains(substr)){
				ret++;
				break;
			}
		}
	}

	return ret;
}

int CoverModel::searchable_column() const
{
	return 0;
}

QString CoverModel::searchable_string(int idx) const
{
	if(idx < 0 || idx >= albums().count())
	{
		return QString();
	}

	return albums()[idx].name();
}

int CoverModel::id_by_row(int idx)
{
	if(idx < 0 || idx >= albums().count())
	{
		return -1;
	}

	return albums()[idx].id;
}

Location CoverModel::cover(const IndexSet& indexes) const
{
	if(indexes.size() != 1){
		return Location::invalid_location();
	}

	int idx = indexes.first();
	if(idx < 0 || idx >= albums().count() ){
		return Location::invalid_location();
	}

	QString hash = get_hash( albums().at(idx) );
	if(!m->cover_locations.contains(hash)){
		return Location::invalid_location();
	}

	return m->cover_locations[hash];
}


Qt::ItemFlags CoverModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags ret = ItemModel::flags(index);

	int row = index.row();
	int column = index.column();

	int max_column = columnCount();
	if(row == rowCount() - 1)
	{
		max_column = albums().size() % columnCount();
	}

	if(column >= max_column || column < 0 || row < 0)
	{
		ret &= ~Qt::ItemIsSelectable;
		ret &= ~Qt::ItemIsEnabled;
		ret &= ~Qt::ItemIsDragEnabled;
	}

	return ret;
}


const MetaDataList& Library::CoverModel::mimedata_tracks() const
{
	return library()->tracks();
}

const AlbumList& CoverModel::albums() const
{
	const AbstractLibrary* al = library();
	const AlbumList& a = al->albums();
	return a;
}

const IndexSet& CoverModel::selections() const
{
	return library()->selected_albums();
}

int CoverModel::zoom() const
{
	return m->zoom;
}

void CoverModel::set_zoom(int zoom, const QSize& view_size)
{
	m->zoom = zoom;

	int columns = (view_size.width() / m->item_size().width());
	if(columns > 0){
		m->columns = columns;
		refresh_data();
	}
}

void CoverModel::reload()
{
	m->pixmaps.clear();
	refresh_data();
}

