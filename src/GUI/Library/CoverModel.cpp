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
#include "Components/Covers/CoverChangeNotifier.h"

#include "Utils/MetaData/Album.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"
#include "Utils/Logger/Logger.h"

#include "GUI/Utils/GuiUtils.h"

#include <QFontMetrics>
#include <QApplication>
#include <QStringList>
#include <QPixmap>
#include <QThread>
#include <QMainWindow>

#include <atomic>
#include <mutex>

using Cover::Location;
using Cover::Lookup;
using Library::CoverModel;

using Hash=AlbumCoverFetchThread::Hash;

struct CoverModel::Private
{
	AlbumCoverFetchThread*		cover_thread=nullptr;
	QHash<Hash, QPixmap>		original_pixmaps;
	QHash<Hash, QPixmap>		pixmaps;
	QHash<Hash, QPixmap>		scaled_pixmaps;
	QHash<Hash, QModelIndex>	indexes;
	QHash<Hash, bool>			valid_hashes;
	QSize						item_size;
	std::mutex					refresh_mtx;

	int	old_row_count;
	int	old_column_count;
	int zoom;
	int columns;

	Private(QObject* parent) :
		old_row_count(0),
		old_column_count(0),
		columns(10)
	{
		cover_thread = new AlbumCoverFetchThread(parent);
		zoom = Settings::instance()->get<Set::Lib_CoverZoom>();

		reset_valid_hashes();
	}

	~Private()
	{
		if(cover_thread)
		{
			cover_thread->stop();
			cover_thread->wait();
		}
	}

	QPixmap get_pixmap(const Hash& hash)
	{
		Qt::TransformationMode mode = Qt::SmoothTransformation;
		QPixmap p = pixmaps[hash];
		if(p.isNull())
		{
			QString invalid_path = Cover::Location::invalid_location().cover_path();
			return QPixmap(invalid_path).scaled(zoom, zoom, Qt::KeepAspectRatio, mode);
		}

		else
		{
			QPixmap ret = p.scaled(zoom, zoom, Qt::KeepAspectRatio, mode);
			scaled_pixmaps[hash] = ret;
			return ret;
		}

		return p;
	}

	void insert_pixmap(Hash hash, const QString& path)
	{
		pixmaps[hash] = QPixmap(path);
	}

	void reset_valid_hashes()
	{
		const QList<Hash> keys = valid_hashes.keys();
		for(const Hash& key : valid_hashes.keys()){
			valid_hashes[key] = false;
		}
	}
};



CoverModel::CoverModel(QObject* parent, AbstractLibrary* library) :
	ItemModel(parent, library)
{
	m = Pimpl::make<Private>(this);

	Cover::ChangeNotfier* ccn = Cover::ChangeNotfier::instance();
	connect(ccn, &Cover::ChangeNotfier::sig_covers_changed, this, &CoverModel::reload);

	connect(library, &AbstractLibrary::sig_all_albums_loaded, this, &CoverModel::refresh_data);

	connect(m->cover_thread, &AlbumCoverFetchThread::sig_next, this, &CoverModel::next_hash);
	connect(m->cover_thread, &QObject::destroyed, this, [=](){
		m->cover_thread = nullptr;
	});

	connect(m->cover_thread, &QThread::finished, this, [=](){
		sp_log(Log::Warning, this) << "Cover Thread finished";
		m->cover_thread = nullptr;
	});

	Set::listen<Set::Lib_CoverShowArtist>(this, &CoverModel::show_artists_changed);

	m->cover_thread->start();
}

CoverModel::~CoverModel() {}

int CoverModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);

	if(columnCount() == 0){
		return 0;
	}

	return (albums().count() / columnCount()) + 1;
}

int CoverModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return m->columns;
}


void CoverModel::refresh_data()
{
	LOCK_GUARD(m->refresh_mtx);

	int old_columns = m->old_column_count;
	int old_rows = m->old_row_count;

	int new_rows = rowCount();
	int new_columns = columnCount();

	if((new_rows == old_rows) && (new_columns == old_columns))
	{
		return;
	}

	if(new_rows > old_rows)	{
		add_rows(old_rows, new_rows - old_rows);
	}

	if(new_columns > old_columns) {
		add_columns(old_columns, new_columns - old_columns);
	}

	if(new_columns < old_columns) {
		remove_columns(new_columns, old_columns - new_columns);
	}

	if(new_rows < old_rows) {
		remove_rows(new_rows, old_rows - new_rows);
	}
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

	const AlbumList& a = this->albums();

	int lin_idx = (index.row() *  columnCount()) + index.column();
	if(lin_idx >= a.count()){
		return QVariant();
	}

	const Album& album = a[lin_idx];

	switch(role)
	{
		case Qt::DisplayRole:
			{
				QString name = album.name();
				if(name.trimmed().isEmpty()){
					name = Lang::get(Lang::None);
				}
				return name;
			}

		case Qt::TextAlignmentRole:
			return static_cast<int>(Qt::AlignHCenter | Qt::AlignTop);

		case Qt::DecorationRole:
			{
				Hash hash = AlbumCoverFetchThread::get_hash(album);
				m->indexes[hash] = index;

				if(m->scaled_pixmaps.contains(hash))
				{
					return m->scaled_pixmaps[hash];
				}

				else if(m->pixmaps.contains(hash))
				{
					if(m->valid_hashes[hash] == false)
					{
						m->cover_thread->add_album(album);
					}

					return m->get_pixmap(hash);
				}

				else
				{
					m->cover_thread->add_album(album);
					return m->get_pixmap(hash);
				}
			}

		case Qt::SizeHintRole:
			return m->item_size;

		case Qt::UserRole:
		{
			QString artist;

			if(Settings::instance()->get<Set::Lib_CoverShowArtist>())
			{
				if(album.album_artists().isEmpty())
				{
					if(!album.artists().isEmpty())
					{
						artist = album.artists().first();
					}
				}

				else
				{
					artist = album.album_artists().first();
				}
			}

			return artist;
		}
		default:
			return QVariant();
	}

	return QVariant();
}



struct CoverLookupUserData
{
	Hash hash;
	Location cl;
	QModelIndex idx;
	AlbumCoverFetchThread* acft=nullptr;
};

static std::mutex mtx1, mtx2;
void CoverModel::next_hash()
{
	AlbumCoverFetchThread* acft = dynamic_cast<AlbumCoverFetchThread*>(sender());
	if(!acft){
		return;
	}

	AlbumCoverFetchThread::HashLocationPair hlp = acft->take_current_location();
	if(hlp.first.isEmpty()){
		return;
	}

	Hash hash = hlp.first;
	Location cl = hlp.second;

	CoverLookupUserData* d = new CoverLookupUserData();
	{
		d->hash = hash;
		d->cl = cl;
		d->idx =  m->indexes[hash];
		d->acft = acft;
	}

	Lookup* clu = new Lookup(this, 1);
	connect(clu, &Lookup::sig_finished, this, &CoverModel::cover_lookup_finished);

	clu->set_user_data(d);
	bool b = clu->fetch_cover(cl);
	if(!b)
	{
		clu->deleteLater();
		acft->done(hash);
	}
}


void CoverModel::cover_lookup_finished(bool success)
{
	Lookup* clu = static_cast<Lookup*>(sender());
	CoverLookupUserData* d = static_cast<CoverLookupUserData*>(clu->take_user_data());

	if(d)
	{
		if(success)
		{
			LOCK_GUARD(mtx1)

			QList<QPixmap> pixmaps = clu->pixmaps();
			if(!pixmaps.isEmpty())
			{
				m->pixmaps[d->hash] = pixmaps.first();
				m->scaled_pixmaps[d->hash] = m->get_pixmap(d->hash);
			}

			emit dataChanged(d->idx, d->idx);
		}

		{
			LOCK_GUARD(mtx2)
			m->valid_hashes[d->hash] = success;
		}

		d->acft->done(d->hash);

		delete d; d=nullptr;
	}

	clu->deleteLater();
}


QModelIndexList CoverModel::search_results(const QString& substr)
{
	QModelIndexList ret;

	const AlbumList& a = albums();
	const int n_albums = a.count();

	for(int i=0; i<n_albums; i++)
	{
		QString title = searchable_string(i);
		title = Library::Utils::convert_search_string(title, search_mode());

		if(title.contains(substr))
		{
			ret << this->index(i / columnCount(), i % columnCount());
			continue;
		}

		const QStringList artists = a[i].artists();
		for(const QString& artist : artists)
		{
			QString cvt_artist = Library::Utils::convert_search_string(artist, search_mode());

			if(cvt_artist.contains(substr)){
				ret << this->index(i / columnCount(), i % columnCount());
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
	const AlbumList& a = albums();
	if(idx < 0 || idx >= a.count())
	{
		return QString();
	}

	return a[idx].name();
}

int CoverModel::id_by_index(int idx) const
{
	const AlbumList& a = albums();
	if(idx < 0 || idx >= a.count())
	{
		return -1;
	}

	return a[idx].id;
}

Location CoverModel::cover(const IndexSet& indexes) const
{
	if(indexes.size() != 1){
		return Location::invalid_location();
	}

	const AlbumList& albums = this->albums();

	int idx = indexes.first();
	if(idx < 0 || idx >= albums.count()){
		return Location::invalid_location();
	}

	Album album = albums[idx];
	return Cover::Location::cover_location(album);
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
	return library()->albums();
}

const Util::Set<Id>& CoverModel::selections() const
{
	return library()->selected_albums();
}

int CoverModel::zoom() const
{
	return m->zoom;
}

QSize CoverModel::item_size() const
{
	return m->item_size;
}


static QSize calc_item_size(int zoom, QFont font)
{
	int text_height = QFontMetrics(font).height();
	bool show_artist = Settings::instance()->get<Set::Lib_CoverShowArtist>();
	if(show_artist)
	{
		text_height = 2 * text_height;
	}

	text_height = (text_height * 12) / 10;

	int width = std::max(((zoom * 115) / 100), zoom + 20);
	int height = width + text_height;

	return QSize(width, height);
}

void CoverModel::set_zoom(int zoom, const QSize& view_size)
{
	m->scaled_pixmaps.clear();
	m->zoom = zoom;
	m->item_size = calc_item_size(zoom, Gui::Util::main_window()->font());

	int columns = (view_size.width() / m->item_size.width());
	if(columns > 0)
	{
		m->columns = columns;
		refresh_data();
		emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {Qt::SizeHintRole});
	}
}

void CoverModel::show_artists_changed()
{
	m->item_size = calc_item_size(m->zoom, Gui::Util::main_window()->font());
}

void CoverModel::reload()
{
	m->cover_thread->pause();
	m->cover_thread->clear();

	m->scaled_pixmaps.clear();
	m->pixmaps.clear();
	m->indexes.clear();
	m->reset_valid_hashes();
	clear();

	emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
	m->cover_thread->resume();
}

void CoverModel::clear()
{
	m->cover_thread->pause();
	m->cover_thread->clear();

	m->scaled_pixmaps.clear();
	m->pixmaps.clear();
	m->indexes.clear();
	m->reset_valid_hashes();

	m->cover_thread->resume();
}

