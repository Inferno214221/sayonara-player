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

#include "Utils/Set.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/GuiUtils.h"

#include <QApplication>
#include <QFontMetrics>
#include <QStringList>
#include <QPixmap>
#include <QMimeData>

#include <mutex>

using Cover::Location;
using Cover::Lookup;
using Library::CoverModel;
using Library::AlbumCoverFetchThread;

using Hash = AlbumCoverFetchThread::Hash;
using HashSet = Util::Set<Hash>;

namespace
{
	std::mutex refreshMtx;

	bool albumMatchesString(const Album& album, const QString& substr, ::Library::SearchModeMask searchMode)
	{
		const auto title = Library::Utils::convertSearchstring(album.name(), searchMode);
		if(title.contains(substr))
		{
			return true;
		}

		else
		{
			const auto hasMatchingArtist = Util::Algorithm::contains(album.artists(), [&](const auto& artist) {
				const auto convertedArtist = Library::Utils::convertSearchstring(artist, searchMode);
				return (convertedArtist.contains(substr));
			});

			if(hasMatchingArtist)
			{
				return true;
			}
		}

		return false;
	}
}

struct CoverModel::Private
{
	public:
		CoverViewPixmapCache coverCache;
		AlbumCoverFetchThread* coverThread;

		QHash<Hash, QModelIndex> hashIndexMap;
		HashSet invalidHashes;
		QSize itemSize;

		int oldRowCount {0};
		int oldColumnCount {0};
		int maxColumns {10};
		int zoom {GetSetting(Set::Lib_CoverZoom)};

		explicit Private(QObject* parent) :
			coverThread {new AlbumCoverFetchThread(parent)} {}

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
	ItemModel(0, parent, library)
{
	m = Pimpl::make<Private>(this);

	auto* coverChangeNotifier = Cover::ChangeNotfier::instance();
	connect(coverChangeNotifier, &Cover::ChangeNotfier::sigCoversChanged, this, &CoverModel::reload);

	connect(library, &AbstractLibrary::sigAllAlbumsLoaded, this, &CoverModel::refreshData);

	connect(m->coverThread, &AlbumCoverFetchThread::sigNext, this, &CoverModel::nextHash);
	connect(m->coverThread, &QObject::destroyed, this, [=]() {
		m->coverThread = nullptr;
	});

	ListenSetting(Set::Lib_CoverShowArtist, CoverModel::showArtistsChanged);

	m->coverThread->start();
}

CoverModel::~CoverModel() = default;

QVariant CoverModel::data(const QModelIndex& index, int role) const
{
	const auto& albums = this->albums();

	const auto linearIndex = (index.row() * columnCount()) + index.column();
	if(!index.isValid() || !Util::between(linearIndex, albums))
	{
		return QVariant();
	}

	const auto& album = albums[linearIndex];

	switch(role)
	{
		case CoverModel::AlbumRole:
			return (album.name().trimmed().isEmpty())
			       ? Lang::get(Lang::UnknownAlbum)
			       : album.name();

		case CoverModel::ArtistRole:
			return (GetSetting(Set::Lib_CoverShowArtist))
			       ? album.albumArtist()
			       : QString {};

		case CoverModel::CoverRole:
		{
			const auto hash = AlbumCoverFetchThread::getHash(album);
			m->hashIndexMap[hash] = index;

			if(m->coverCache.hasPixmap(hash))
			{
				const auto pixmap = m->coverCache.pixmap(hash);
				if(m->coverCache.isOutdated(hash))
				{
					m->coverThread->addAlbum(album);
				}

				return pixmap;
			}

			if(!m->invalidHashes.contains(hash))
			{
				spLog(Log::Develop, this) << "Need to fetch cover for " << hash;
				m->coverThread->addAlbum(album);
			}

			return m->coverCache.invalidPixmap();
		}

		case Qt::TextAlignmentRole:
			return static_cast<int>(Qt::AlignHCenter | Qt::AlignTop);

		case Qt::SizeHintRole:
			return m->itemSize;

		case Qt::ToolTipRole:
		{
			const auto artistName = (album.albumArtist().trimmed().isEmpty())
			                        ? Lang::get(Lang::UnknownArtist)
			                        : album.albumArtist();

			const auto albumName = (album.name().trimmed().isEmpty())
			                       ? Lang::get(Lang::UnknownAlbum)
			                       : album.name();

			return QString("<b>%1</b><br>%2")
				.arg(artistName)
				.arg(albumName);
		}

		default:
			return QVariant {};
	}
}

void CoverModel::nextHash()
{
	if(!m->coverThread)
	{
		return;
	}

	const auto[hash, location] = m->coverThread->takeCurrentLookup();
	if(!hash.isEmpty() && location.isValid())
	{
		auto* coverLookup = new Lookup(location, 1, nullptr);

		coverLookup->setUserData(hash);
		connect(coverLookup, &Lookup::sigFinished, this, &CoverModel::coverLookupFinished);

		coverLookup->start();
	}
}

void CoverModel::coverLookupFinished(bool success)
{
	auto* coverLookup = dynamic_cast<Lookup*>(sender());

	const auto hash = coverLookup->userData<Hash>();
	const auto pixmaps = (success)
	                     ? coverLookup->pixmaps()
	                     : QList<QPixmap> {};

	coverLookup->deleteLater();

	if(!pixmaps.isEmpty())
	{
		m->coverCache.addPixmap(hash, pixmaps.first());
	}

	else
	{
		m->invalidHashes.insert(hash);
	}

	m->coverThread->removeHash(hash);

	const auto index = m->hashIndexMap.value(hash);
	emit dataChanged(index, index, {Qt::DecorationRole});
}

QModelIndexList CoverModel::searchResults(const QString& substr)
{
	QModelIndexList matchingIndexes;

	const auto& albums = this->albums();

	int i = 0;
	for(auto it = albums.begin(); it != albums.end(); it++, i++)
	{
		if(albumMatchesString(*it, substr, searchMode()))
		{
			matchingIndexes << this->index(i / columnCount(), i % columnCount());
		}
	}

	return matchingIndexes;
}

int CoverModel::searchableColumn() const
{
	return 0;
}

QString CoverModel::searchableString(int index) const
{
	const auto& albums = this->albums();
	return (Util::between(index, albums))
	       ? albums[index].name()
	       : QString {};
}

int CoverModel::mapIndexToId(int index) const
{
	const auto& albums = this->albums();
	return (Util::between(index, albums))
	       ? albums[index].id()
	       : -1;
}

Location CoverModel::cover(const QModelIndexList& indexes) const
{
	if(indexes.size() != 1)
	{
		return Location::invalidLocation();
	}

	const auto& albums = this->albums();
	const auto& firstIndex = indexes.first();
	const auto linearIndex = (firstIndex.row() * columnCount() + firstIndex.column());

	return (Util::between(linearIndex, albums))
	       ? Cover::Location::coverLocation(albums[linearIndex])
	       : Location::invalidLocation();
}

Qt::ItemFlags CoverModel::flags(const QModelIndex& index) const
{
	auto itemFlags = ItemModel::flags(index);

	const auto linearIndex = (index.row() * columnCount() + index.column());
	if(!Util::between(linearIndex, albums()))
	{
		itemFlags &= ~Qt::ItemIsSelectable;
		itemFlags &= ~Qt::ItemIsEnabled;
		itemFlags &= ~Qt::ItemIsDragEnabled;
	}

	return itemFlags;
}

const MetaDataList& Library::CoverModel::selectedMetadata() const
{
	return library()->tracks();
}

const AlbumList& CoverModel::albums() const
{
	return library()->albums();
}

QSize CoverModel::itemSize() const
{
	return m->itemSize;
}

int CoverModel::zoom() const
{
	return m->zoom;
}

static QSize calcItemSize(int zoom, const QFont& font)
{
	const auto showArtist = GetSetting(Set::Lib_CoverShowArtist);

	const auto lines = (showArtist) ? 2 : 1;
	const auto lineHeight = QFontMetrics(font).height();

	const auto textHeight = (lines * lineHeight * 1.2);
	const auto itemWidth = std::max<double>(zoom * 1.15, zoom + 20.0);

	const auto height = itemWidth + textHeight;

	return QSize(
		static_cast<int>(itemWidth),
		static_cast<int>(height)
	);
}

void CoverModel::setZoom(int zoom, const QSize& viewSize)
{
	SetSetting(Set::Lib_CoverZoom, zoom);

	m->zoom = zoom;
	m->itemSize = calcItemSize(zoom, QApplication::font());

	const auto columns = (viewSize.width() / m->itemSize.width());
	if(columns > 0)
	{
		const auto visibleRows = (viewSize.height() / m->itemSize.height()) + 1;
		const auto visibleItems = visibleRows * columns;

		m->maxColumns = columns;
		m->coverCache.setCacheSize(visibleItems * 3);

		refreshData();
	}
}

void CoverModel::showArtistsChanged()
{
	m->itemSize = calcItemSize(m->zoom, QApplication::font());
}

void CoverModel::reload()
{
	m->coverCache.setAllOutdated();
	clear();

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void CoverModel::clear()
{
	m->invalidHashes.clear();
	m->coverThread->clear();
	m->hashIndexMap.clear();
}

int CoverModel::rowCount([[maybe_unused]] const QModelIndex& index) const
{
	const auto columns = columnCount();
	return (columns != 0)
	       ? (albums().count() + (columns - 1)) / columns
	       : 0;
}

int CoverModel::columnCount(const QModelIndex&) const
{
	return std::min(m->maxColumns, albums().count());
}

void CoverModel::refreshData()
{
	[[maybe_unused]] std::lock_guard<std::mutex> lockGuard(refreshMtx);

	const auto oldColumns = m->oldColumnCount;
	const auto oldRows = m->oldRowCount;

	const auto newRows = rowCount();
	const auto newColumns = columnCount();

	if((newRows == oldRows) && (newColumns == oldColumns))
	{
		return;
	}

	if(newRows > oldRows)
	{
		insertRows(oldRows, newRows - oldRows);
	}

	if(newColumns > oldColumns)
	{
		insertColumns(oldColumns, newColumns - oldColumns);
	}

	if(newColumns < oldColumns)
	{
		removeColumns(newColumns, oldColumns - newColumns);
	}

	if(newRows < oldRows)
	{
		removeRows(newRows, oldRows - newRows);
	}
}

bool CoverModel::insertRows(int row, int count, [[maybe_unused]] const QModelIndex& parent)
{
	beginInsertRows(QModelIndex(), row, row + count - 1);
	m->oldRowCount += count;
	endInsertRows();

	return true;
}

bool CoverModel::removeRows(int row, int count, [[maybe_unused]] const QModelIndex& parent)
{
	beginRemoveRows(QModelIndex(), row, row + count - 1);
	m->oldRowCount -= count;
	endRemoveRows();

	return true;
}

bool CoverModel::insertColumns(int column, int count, [[maybe_unused]] const QModelIndex& parent)
{
	beginInsertColumns(QModelIndex(), column, column + count - 1);
	m->oldColumnCount += count;
	endInsertColumns();

	return true;
}

bool CoverModel::removeColumns(int column, int count, [[maybe_unused]] const QModelIndex& parent)
{
	beginRemoveColumns(QModelIndex(), column, column + count - 1);
	m->oldColumnCount -= count;
	endRemoveColumns();

	return true;
}

QModelIndex Library::CoverModel::index(int row, int column, const QModelIndex& parent) const
{
	const auto linearIndex = (row * columnCount()) + column;
	return (Util::between(linearIndex, albums()))
	       ? ItemModel::index(row, column, parent)
	       : QModelIndex();
}
