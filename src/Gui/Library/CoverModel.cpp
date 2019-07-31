/* AlbumCoverModel.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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
#include "Utils/AlbumCoverFetchThread.h"
#include "Utils/CoverViewPixmapCache.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookup.h"
#include "Components/Covers/CoverChangeNotifier.h"

#include "Utils/MetaData/Album.h"
#include "Utils/Mutex.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/GuiUtils.h"

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
using Library::AlbumCoverFetchThread;

using Hash=AlbumCoverFetchThread::Hash;
using HashSet=Util::Set<Hash>;

struct CoverModel::Private
{
private:

public:
	CoverViewPixmapCache*		cvpc=nullptr;
	AlbumCoverFetchThread*		cover_thread=nullptr;

	QHash<Hash, QModelIndex>	indexes;
	HashSet						invalid_hashes;
	QSize						item_size;

	std::mutex					refresh_mtx;

	int	old_row_count;
	int	old_column_count;
	int columns;
	int	clus_running;
	int	zoom;

	Private(QObject* parent) :
		old_row_count(0),
		old_column_count(0),
		columns(10),
		clus_running(0),
		zoom(100)
	{
		cover_thread = new AlbumCoverFetchThread(parent);
	}

	~Private()
	{
		if(cover_thread)
		{
			cover_thread->stop();
			cover_thread->wait();
		}
	}
};



CoverModel::CoverModel(QObject* parent, AbstractLibrary* library) :
	ItemModel(parent, library)
{
	m = Pimpl::make<Private>(this);
	m->cvpc = new CoverViewPixmapCache();

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

	ListenSetting(Set::Lib_CoverShowArtist, CoverModel::show_artists_changed);

	m->cover_thread->start();
}

CoverModel::~CoverModel() {}

static QString get_artist(const Album& album)
{
	QStringList artists = album.album_artists();
	artists.removeAll("");

	if(artists.isEmpty())
	{
		artists = album.artists();
		artists.removeAll("");
	}

	if(artists.isEmpty()){
		return Lang::get(Lang::UnknownArtist);
	}

	if(artists.size() == 1){
		return artists.first();
	}

	return Lang::get(Lang::VariousArtists);
}

QVariant CoverModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid()) {
		return QVariant();
	}

	const AlbumList& albums = this->albums();

	int lin_idx = (index.row() *  columnCount()) + index.column();
	if(lin_idx >= albums.count()){
		return QVariant();
	}

	const Album& album = albums[lin_idx];
	switch(role)
	{

		case CoverModel::AlbumRole:
		case Qt::DisplayRole:
			{
				QString name = album.name();
				if(name.trimmed().isEmpty()){
					name = Lang::get(Lang::UnknownAlbum);
				}
				return name;
			}

		case Qt::TextAlignmentRole:
			return static_cast<int>(Qt::AlignHCenter | Qt::AlignTop);

		case Qt::DecorationRole:
			{
				Hash hash = AlbumCoverFetchThread::get_hash(album);
				m->indexes[hash] = index;

				if(m->cvpc->has_pixmap(hash))
				{
					return m->cvpc->pixmap(hash);
				}

				if(m->invalid_hashes.contains(hash)){
					return m->cvpc->invalid_pixmap();
				}

				sp_log(Log::Develop, this) << "Need to fetch cover for " << hash;
				m->cover_thread->add_album(album);

				return m->cvpc->invalid_pixmap();
			}

		case Qt::SizeHintRole:
			return m->item_size;

		case CoverModel::ArtistRole:
			if(GetSetting(Set::Lib_CoverShowArtist))
			{
				return get_artist(album);
			}

			return QString();

		case Qt::ToolTipRole:
			return QString("<b>%1</b><br>%2")
					.arg(get_artist(album))
					.arg(album.name());

		default:
			return QVariant();
	}
}


struct CoverLookupUserData
{
	Hash hash;
	Location cl;
	AlbumCoverFetchThread* acft=nullptr;
};

void CoverModel::next_hash()
{
	AlbumCoverFetchThread* acft = dynamic_cast<AlbumCoverFetchThread*>(sender());
	if(!acft){
		return;
	}

	AlbumCoverFetchThread::HashLocationPair hlp = acft->take_current_lookup();
	if(hlp.first.isEmpty() || !hlp.second.is_valid()){
		return;
	}

	sp_log(Log::Crazy, this) << "Status cover fetch thread:";
	sp_log(Log::Crazy, this) << "  Lookups ready: " << acft->lookups_ready();
	sp_log(Log::Crazy, this) << "  Unprocessed hashes: " << acft->unprocessed_hashes();
	sp_log(Log::Crazy, this) << "  Queued hashes: " << acft->queued_hashes();


	Hash hash = hlp.first;
	Location cl = hlp.second;

	Lookup* clu = new Lookup(cl, 1, nullptr);

	CoverLookupUserData* d = new CoverLookupUserData();
	{
		d->hash = hash;
		d->cl = clu->cover_location();
		d->acft = acft;
	}

	clu->set_user_data(d);

	connect(clu, &Lookup::sig_finished, this, &CoverModel::cover_lookup_finished);


	m->clus_running++;
	sp_log(Log::Crazy, this) << "CLU started: " << m->clus_running << ", " << d->hash;
	clu->start();
}

void CoverModel::cover_lookup_finished(bool success)
{
	Lookup* clu = static_cast<Lookup*>(sender());
	CoverLookupUserData* d = static_cast<CoverLookupUserData*>(clu->user_data());

	QList<QPixmap> pixmaps;
	if(success)
	{
		pixmaps = clu->pixmaps();
	}

	if(!pixmaps.isEmpty())
	{
		QPixmap pm(pixmaps.first());
		m->cvpc->add_pixmap(d->hash, pm);
	}

	else
	{
		m->invalid_hashes.insert(d->hash);
	}

	m->clus_running--;
	sp_log(Log::Crazy, this) << "CLU finished: " << m->clus_running << ", " << d->hash;
	d->acft->done(d->hash);

	clu->set_user_data(nullptr);

	delete clu; clu = nullptr;

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {Qt::DecorationRole});
}

void CoverModel::cover_ready(const QString& hash)
{
	QModelIndex index = m->indexes.value(hash);
	emit dataChanged(index, index);
}

QModelIndexList CoverModel::search_results(const QString& substr)
{
	QModelIndexList ret;

	const AlbumList& albums = this->albums();

	int i=0;
	for(auto it=albums.begin(); it != albums.end(); it++, i++)
	{
		QString title = searchable_string(i);
		title = Library::Utils::convert_search_string(title, search_mode());

		if(title.contains(substr))
		{
			ret << this->index(i / columnCount(), i % columnCount());
			continue;
		}

		const QStringList artists = it->artists();
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
	const AlbumList& albums = this->albums();
	if(idx < 0 || idx >= albums.count())
	{
		return QString();
	}

	return albums[idx].name();
}

int CoverModel::id_by_index(int idx) const
{
	const AlbumList& albums = this->albums();
	if(idx < 0 || idx >= albums.count())
	{
		return -1;
	}

	return albums[idx].id;
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
	return Cover::Location::xcover_location(album);
}


Qt::ItemFlags CoverModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags ret = ItemModel::flags(index);

	int row = index.row();
	int column = index.column();

	int max_column = columnCount();
	if(row == rowCount() - 1)
	{
		max_column = albums().count() % columnCount();
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


QSize CoverModel::item_size() const
{
	return m->item_size;
}

int CoverModel::zoom() const
{
	return m->zoom;
}


static QSize calc_item_size(int zoom, QFont font)
{
	int text_height = QFontMetrics(font).height();
	bool show_artist = GetSetting(Set::Lib_CoverShowArtist);
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
	m->zoom = zoom;
	m->item_size = calc_item_size(zoom, Gui::Util::main_window()->font());

	int columns = (view_size.width() / m->item_size.width());
	if(columns > 0)
	{
		m->columns = columns;

		int visible_rows = (view_size.height() / m->item_size.height()) + 1;
		m->cvpc->set_cache_size(visible_rows * columns * 3);

		refresh_data();
	}
}

void CoverModel::show_artists_changed()
{
	m->item_size = calc_item_size(m->zoom, Gui::Util::main_window()->font());
}

void CoverModel::reload()
{
	m->cvpc->clear();
	clear();

	emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
}

void CoverModel::clear()
{
	m->invalid_hashes.clear();
	m->cover_thread->clear();
	m->indexes.clear();
}


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

