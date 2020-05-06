/* AlbumCoverModel.cpp */

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

#include "CoverModel.h"
#include "AlbumCoverFetchThread.h"
#include "CoverViewPixmapCache.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookup.h"
#include "Components/Covers/CoverChangeNotifier.h"

#include "Utils/Mutex.h"
#include "Utils/Set.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/GuiUtils.h"

#include <QFontMetrics>
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
	CoverViewPixmapCache*		coverCache=nullptr;
	AlbumCoverFetchThread*		coverThread=nullptr;

	QHash<Hash, QModelIndex>	hashIndexMap;
	HashSet						invalidHashes;
	QSize						itemSize;

	std::mutex					refreshMtx;

	int	oldRowCount;
	int	oldColumnCount;
	int columns;
	int	zoom;

	Private(QObject* parent) :
		oldRowCount(0),
		oldColumnCount(0),
		columns(10),
		zoom(100)
	{
		coverThread = new AlbumCoverFetchThread(parent);
		coverCache = new CoverViewPixmapCache();
	}

	~Private()
	{
		if(coverThread)
		{
			coverThread->stop();
			coverThread->wait();
		}
	}
};


CoverModel::CoverModel(QObject* parent, AbstractLibrary* library) :
	ItemModel(parent, library)
{
	m = Pimpl::make<Private>(this);

	auto* coverChangeNotifier = Cover::ChangeNotfier::instance();
	connect(coverChangeNotifier, &Cover::ChangeNotfier::sigCoversChanged, this, &CoverModel::reload);

	connect(library, &AbstractLibrary::sigAllAlbumsLoaded, this, &CoverModel::refreshData);

	connect(m->coverThread, &AlbumCoverFetchThread::sigNext, this, &CoverModel::nextHash);
	connect(m->coverThread, &QObject::destroyed, this, [=](){
		m->coverThread = nullptr;
	});

	connect(m->coverThread, &QThread::finished, this, [=](){
		spLog(Log::Warning, this) << "Cover Thread finished";
		m->coverThread = nullptr;
	});

	ListenSetting(Set::Lib_CoverShowArtist, CoverModel::showArtistsChanged);

	m->coverThread->start();
}

CoverModel::~CoverModel() = default;

QVariant CoverModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid()) {
		return QVariant();
	}

	const AlbumList& albums = this->albums();

	int linearIndex = (index.row() *  columnCount()) + index.column();
	if(linearIndex < 0 || linearIndex >= albums.count()){
		return QVariant();
	}

	const Album& album = albums[linearIndex];

	if(role == CoverModel::AlbumRole)
	{
		QString name = album.name();
		if(name.trimmed().isEmpty()){
			name = Lang::get(Lang::UnknownAlbum);
		}
		return name;
	}

	else if(role == CoverModel::ArtistRole)
	{
		if(GetSetting(Set::Lib_CoverShowArtist))
		{
			return album.albumArtist();
		}

		return QString();
	}

	else if(role == CoverModel::CoverRole)
	{
		Hash hash = AlbumCoverFetchThread::getHash(album);
		m->hashIndexMap[hash] = index;

		if(m->coverCache->hasPixmap(hash))
		{
			QPixmap pm = m->coverCache->pixmap(hash);
			if(m->coverCache->isOutdated(hash))
			{
				m->coverThread->addAlbum(album);
			}

			return pm;
		}

		if(m->invalidHashes.contains(hash)) {
			return m->coverCache->invalidPixmap();
		}

		spLog(Log::Develop, this) << "Need to fetch cover for " << hash;
		m->coverThread->addAlbum(album);

		return m->coverCache->invalidPixmap();
	}

	else if(role == Qt::DisplayRole)
	{
		return QVariant();
	}

	else if(role == Qt::TextAlignmentRole)
	{
		return int(Qt::AlignHCenter | Qt::AlignTop);
	}

	else if(role == Qt::SizeHintRole)
	{
		return m->itemSize;
	}

	else if(role == Qt::ToolTipRole)
	{
		QString artistName = album.albumArtist();
		if(artistName.trimmed().isEmpty()){
			artistName = Lang::get(Lang::UnknownArtist);
		}

		QString albumName = album.name();
		if(albumName.trimmed().isEmpty()){
			albumName = Lang::get(Lang::UnknownAlbum);
		}

		return QString("<b>%1</b><br>%2")
				.arg(artistName)
				.arg(albumName);
	}

	return QVariant();
}

void CoverModel::nextHash()
{
	auto* fetchThread = dynamic_cast<AlbumCoverFetchThread*>(sender());
	if(!fetchThread){
		return;
	}

	AlbumCoverFetchThread::HashLocationPair hlp = fetchThread->takeCurrentLookup();
	if(hlp.first.isEmpty() || !hlp.second.isValid()){
		return;
	}

	spLog(Log::Crazy, this) << "Status cover fetch thread:";
	spLog(Log::Crazy, this) << "  Lookups ready: " << fetchThread->lookupsReady();
	spLog(Log::Crazy, this) << "  Unprocessed hashes: " << fetchThread->unprocessedHashes();
	spLog(Log::Crazy, this) << "  Queued hashes: " << fetchThread->queuedHashes();

	Hash hash = hlp.first;
	Location cl = hlp.second;

	auto* coverLookup = new Lookup(cl, 1, nullptr);
	auto* data = new Hash(hash);

	coverLookup->setUserData(data);
	connect(coverLookup, &Lookup::sigFinished, this, &CoverModel::coverLookupFinished);

	coverLookup->start();
}

void CoverModel::coverLookupFinished(bool success)
{
	auto* coverLookup = static_cast<Lookup*>(sender());
	auto* data = static_cast<Hash*>(coverLookup->userData());

	Hash hash = *data;

	QList<QPixmap> pixmaps;
	if(success) {
		pixmaps = coverLookup->pixmaps();
	}

	if(!pixmaps.isEmpty())
	{
		QPixmap pm(pixmaps.first());
		m->coverCache->addPixmap(hash, pm);
	}

	else {
		m->invalidHashes.insert(hash);
	}

	m->coverThread->removeHash(hash);

	const QModelIndex index = m->hashIndexMap.value(hash);
	emit dataChanged(index, index, {Qt::DecorationRole});

	delete coverLookup; coverLookup = nullptr;
}

QModelIndexList CoverModel::searchResults(const QString& substr)
{
	QModelIndexList ret;

	const AlbumList& albums = this->albums();

	int i=0;
	for(auto it=albums.begin(); it != albums.end(); it++, i++)
	{
		QString title = searchableString(i);
		title = Library::Utils::convertSearchstring(title, searchMode());

		if(title.contains(substr))
		{
			ret << this->index(i / columnCount(), i % columnCount());
			continue;
		}

		const QStringList artists = it->artists();
		for(const QString& artist : artists)
		{
			QString convertedArtist = Library::Utils::convertSearchstring(artist, searchMode());

			if(convertedArtist.contains(substr)){
				ret << this->index(i / columnCount(), i % columnCount());
				break;
			}
		}
	}

	return ret;
}

int CoverModel::searchableColumn() const
{
	return 0;
}

QString CoverModel::searchableString(int idx) const
{
	const AlbumList& albums = this->albums();
	if(idx < 0 || idx >= albums.count())
	{
		return QString();
	}

	return albums[idx].name();
}

int CoverModel::mapIndexToId(int idx) const
{
	const AlbumList& albums = this->albums();
	if(idx < 0 || idx >= albums.count())
	{
		return -1;
	}

	return albums[idx].id();
}

Location CoverModel::cover(const IndexSet& indexes) const
{
	if(indexes.size() != 1){
		return Location::invalidLocation();
	}

	const AlbumList& albums = this->albums();

	int idx = indexes.first();
	if(idx < 0 || idx >= albums.count()){
		return Location::invalidLocation();
	}

	Album album = albums[idx];
	return Cover::Location::xcoverLocation(album);
}

Qt::ItemFlags CoverModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags ret = ItemModel::flags(index);

	int row = index.row();
	int column = index.column();

	int maxColumn = columnCount();
	if(row == rowCount() - 1)
	{
		maxColumn = albums().count() % columnCount();
	}

	if(column >= maxColumn || column < 0 || row < 0)
	{
		ret &= ~Qt::ItemIsSelectable;
		ret &= ~Qt::ItemIsEnabled;
		ret &= ~Qt::ItemIsDragEnabled;
	}

	return ret;
}

const MetaDataList& Library::CoverModel::selectedMetadata() const
{
	return library()->tracks();
}

const AlbumList& CoverModel::albums() const
{
	return library()->albums();
}

QSize CoverModel::item_size() const
{
	return m->itemSize;
}

int CoverModel::zoom() const
{
	return m->zoom;
}

static QSize calcItemSize(int zoom, QFont font)
{
	int textHeight = QFontMetrics(font).height();
	bool showArtist = GetSetting(Set::Lib_CoverShowArtist);
	if(showArtist)
	{
		textHeight = 2 * textHeight;
	}

	textHeight = (textHeight * 12) / 10;

	int width = std::max(((zoom * 115) / 100), zoom + 20);
	int height = width + textHeight;

	return QSize(width, height);
}

void CoverModel::setZoom(int zoom, const QSize& viewSize)
{
	m->zoom = zoom;
	m->itemSize = calcItemSize(zoom, Gui::Util::mainWindow()->font());

	int columns = (viewSize.width() / m->itemSize.width());
	if(columns > 0)
	{
		m->columns = columns;

		int visible_rows = (viewSize.height() / m->itemSize.height()) + 1;
		m->coverCache->setCacheSize(visible_rows * columns * 3);

		refreshData();
	}
}

void CoverModel::showArtistsChanged()
{
	m->itemSize = calcItemSize(m->zoom, Gui::Util::mainWindow()->font());
}

void CoverModel::reload()
{
	m->coverCache->setAllOutdated();
	clear();

	emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
}

void CoverModel::clear()
{
	m->invalidHashes.clear();
	m->coverThread->clear();
	m->hashIndexMap.clear();
}

int CoverModel::rowCount(const QModelIndex&) const
{
	if(columnCount() == 0){
		return 0;
	}

	return (albums().count() / columnCount()) + 1;
}

int CoverModel::columnCount(const QModelIndex&) const
{
	return m->columns;
}

void CoverModel::refreshData()
{
	LOCK_GUARD(m->refreshMtx)

	int oldColumns = m->oldColumnCount;
	int oldRows = m->oldRowCount;

	int newRows = rowCount();
	int newColumns = columnCount();

	if((newRows == oldRows) && (newColumns == oldColumns))
	{
		return;
	}

	if(newRows > oldRows)	{
		insertRows(oldRows, newRows - oldRows);
	}

	if(newColumns > oldColumns) {
		insertColumns(oldColumns, newColumns - oldColumns);
	}

	if(newColumns < oldColumns) {
		removeColumns(newColumns, oldColumns - newColumns);
	}

	if(newRows < oldRows) {
		removeRows(newRows, oldRows - newRows);
	}
}

bool CoverModel::insertRows(int row, int count, const QModelIndex& parent)
{
	Q_UNUSED(parent)

	beginInsertRows(QModelIndex(), row, row + count - 1);
	m->oldRowCount += count;
	endInsertRows();

	return true;
}

bool CoverModel::removeRows(int row, int count, const QModelIndex& parent)
{
	Q_UNUSED(parent)

	beginRemoveRows(QModelIndex(), row, row + count - 1);
	m->oldRowCount -= count;
	endRemoveRows();

	return true;
}

bool CoverModel::insertColumns(int column, int count, const QModelIndex& parent)
{
	Q_UNUSED(parent)

	beginInsertColumns(QModelIndex(), column, column + count - 1);
	m->oldColumnCount += count;
	endInsertColumns();

	return true;
}

bool CoverModel::removeColumns(int column, int count, const QModelIndex& parent)
{
	Q_UNUSED(parent)

	beginRemoveColumns(QModelIndex(), column, column + count - 1);
	m->oldColumnCount -= count;
	endRemoveColumns();

	return true;
}

