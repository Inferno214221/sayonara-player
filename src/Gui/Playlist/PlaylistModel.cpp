/* PlaylistItemModel.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookup.h"
#include "Components/Playlist/LocalPathProcessor.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistLibraryInteractor.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Components/Tagging/UserTaggingOperations.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Interfaces/PlaylistInterface.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Tagging/TagWriter.h"
#include "Utils/Utils.h"
#include "Utils/globals.h"

#include <QFont>
#include <QFileInfo>
#include <QUrl>
#include <QHash>
#include <QIcon>
#include <QPixmap>

#include <utility>

namespace Algorithm = Util::Algorithm;
using Playlist::Model;

namespace
{
	constexpr const auto AlbumSearchPrefix = '%';
	constexpr const auto ArtistSearchPrefix = '$';
	constexpr const auto FilenameSearchPrefix = '/';
	constexpr const auto JumpPrefix = ':';

	enum class PlaylistSearchMode
	{
		Artist,
		Album,
		Title,
		Filename,
		Jump
	};

	QString convertEntryLook(const QString& entryLook, const MetaData& md)
	{
		auto ret = entryLook;
		ret.replace(QStringLiteral("*"), QChar(Model::Bold));
		ret.replace(QStringLiteral("'"), QChar(Model::Italic));

		ret.replace(QStringLiteral("%title%"), md.title());
		ret.replace(QStringLiteral("%nr%"), QString::number(md.trackNumber()));
		ret.replace(QStringLiteral("%artist%"), md.artist());
		ret.replace(QStringLiteral("%album%"), md.album());

		if(entryLook.indexOf("%filename%") != -1)
		{
			QFileInfo fi(md.filepath());
			const auto fileName = fi.fileName();
			ret.replace(QStringLiteral("%filename%"), fileName);
		}

		return ret;
	}

	std::pair<PlaylistSearchMode, QString> evaluateSearchString(QString searchString)
	{
		auto playlistSearchMode = PlaylistSearchMode::Title;

		if(searchString.startsWith(ArtistSearchPrefix))
		{
			playlistSearchMode = PlaylistSearchMode::Artist;
			searchString.remove(ArtistSearchPrefix);
		}
		else if(searchString.startsWith(AlbumSearchPrefix))
		{
			playlistSearchMode = PlaylistSearchMode::Album;
			searchString.remove(AlbumSearchPrefix);
		}
		else if(searchString.startsWith(FilenameSearchPrefix))
		{
			playlistSearchMode = PlaylistSearchMode::Filename;
			searchString.remove(FilenameSearchPrefix);
		}
		else if(searchString.startsWith(JumpPrefix))
		{
			playlistSearchMode = PlaylistSearchMode::Jump;
			searchString.remove(JumpPrefix);
		}

		return std::make_pair(playlistSearchMode, searchString.trimmed());
	}

	QString
	calculateSearchKey(const MetaData& track, PlaylistSearchMode playlistSearchMode, Library::SearchModeMask searchMode)
	{
		QString str;
		switch(playlistSearchMode)
		{
			case PlaylistSearchMode::Artist:
				str = track.artist();
				break;
			case PlaylistSearchMode::Album:
				str = track.album();
				break;
			case PlaylistSearchMode::Filename:
				str = QFileInfo(track.filepath()).fileName();
				break;
			default:
				str = track.title();
				break;
		}

		return Library::convertSearchstring(str, searchMode);
	}

	int extractRowFromSearchstring(const QString& searchString, int maxRow)
	{
		auto ok = false;
		const auto line = searchString.toInt(&ok);

		return (ok && (line <= maxRow))
		       ? line : -1;
	}

	QString getAlbumHashFromTrack(const MetaData& track)
	{
		return QString("%1-%2")
			.arg(track.albumId())
			.arg(track.album());
	}

	QString getPodcastTooltip(const MetaData& track)
	{
		if(const auto description = track.customField("description"); !description.isEmpty())
		{
			return QString("%1 - %2<br>%3")
				.arg(track.title())
				.arg(track.artist())
				.arg(description);
		}

		return {};
	}
}

struct Model::Private
{
	QHash<QString, QPixmap> coverLookupMap;
	int oldRowCount {0};
	int dragIndex {-1};
	PlaylistPtr playlist;
	Tagging::UserOperations* uto = nullptr;
	std::shared_ptr<LibraryInteractor> libraryInteractor;

	Private(PlaylistPtr playlistArg, Library::InfoAccessor* libraryAccessor) :
		playlist(std::move(playlistArg)),
		libraryInteractor {std::make_shared<LibraryInteractor>(libraryAccessor)} {}
};

Model::Model(const PlaylistPtr& playlist, Library::InfoAccessor* libraryAccessor, QObject* parent) :
	SearchableTableModel(parent)
{
	m = Pimpl::make<Private>(playlist, libraryAccessor);

	connect(m->playlist.get(), &Playlist::Playlist::sigItemsChanged, this, &Model::playlistChanged);
	connect(m->playlist.get(), &Playlist::Playlist::sigTrackChanged, this, &Model::currentTrackChanged);
	connect(m->playlist.get(), &Playlist::sigBusyChanged, this, &Model::sigBusyChanged);
	connect(m->playlist.get(), &Playlist::sigCurrentScannedFileChanged, this, &Model::sigCurrentScannedFileChanged);

	const auto* coverChangeNotifier = Cover::ChangeNotfier::instance();
	connect(coverChangeNotifier, &Cover::ChangeNotfier::sigCoversChanged, this, &Model::coversChanged);

	ListenSettingNoCall(Set::PL_EntryLook, Model::lookChanged);

	// don't call virtual methods here
	refreshPlaylist(::Playlist::count(*m->playlist), static_cast<int>(ColumnName::NumColumns));
}

Model::~Model() = default;

int Model::rowCount([[maybe_unused]] const QModelIndex& parent) const
{
	return ::Playlist::count(*m->playlist);
}

int Model::columnCount([[maybe_unused]] const QModelIndex& parent) const
{
	return static_cast<int>(ColumnName::NumColumns);
}

QVariant Model::data(const QModelIndex& index, int role) const // NOLINT(readability-function-cognitive-complexity)
{
	const auto row = index.row();
	const auto col = index.column();
	const auto isCurrentTrack = (row == m->playlist->currentTrackIndex());

	if(!Util::between(row, ::Playlist::count(*m->playlist)))
	{
		return {};
	}

	const auto& tracks = m->playlist->tracks();
	const auto& track = tracks[row];

	if(role == Qt::DisplayRole)
	{
		if(col == ColumnName::TrackNumber)
		{
			return (isCurrentTrack)
			       ? QString()
			       : QString("%1.").arg(row + 1);
		}

		if(col == ColumnName::Time)
		{
			auto durationMs = track.durationMs();
			return (durationMs / 1000 <= 0) // NOLINT(readability-magic-numbers)
			       ? QVariant()
			       : Util::msToString(durationMs, QStringLiteral("$M:$S"));
		}
	}

	if(role == Qt::ToolTipRole)
	{
		return (track.radioMode() == RadioMode::Podcast)
		       ? getPodcastTooltip(track)
		       : QString {};
	}

	if(role == Qt::TextAlignmentRole)
	{
		return (col == ColumnName::Description)
		       ? QVariant(Qt::AlignLeft | Qt::AlignVCenter)
		       : QVariant(Qt::AlignRight | Qt::AlignVCenter);
	}

	if(role == Qt::DecorationRole)
	{
		if(col == ColumnName::Cover)
		{
			const auto hash = getAlbumHashFromTrack(track);
			if(!m->coverLookupMap.contains(hash))
			{
				m->coverLookupMap.insert(hash, QPixmap {});
				startCoverLookup(track);
			}

			return QIcon(m->coverLookupMap[hash]);
		}
	}

	if(role == Model::EntryLookRole)
	{
		if(col == ColumnName::Description)
		{
			return convertEntryLook(GetSetting(Set::PL_EntryLook), track);
		}
	}

	if(role == Model::DragIndexRole)
	{
		return (row == m->dragIndex);
	}

	if((role == Model::RatingRole) || (role == Qt::EditRole))
	{
		if(col == ColumnName::Description)
		{
			const auto rating = (m->uto && m->uto->newRating(track.id()) != Rating::Last)
			                    ? m->uto->newRating(track.id())
			                    : track.rating();

			return (track.radioMode() == RadioMode::Off)
			       ? QVariant::fromValue(rating)
			       : QVariant::fromValue(Rating::Last);
		}
	}

	if(role == Model::CurrentPlayingRole)
	{
		return isCurrentTrack;
	}

	return {};
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role)
{
	const auto validEdit = (role == Qt::EditRole) && index.isValid();
	if(validEdit)
	{
		const auto row = index.row();
		changeRating({row}, value.value<Rating>());
	}

	return validEdit;
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const
{
	if(!index.isValid())
	{
		return (Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
	}

	const auto row = index.row();
	const auto& tracks = m->playlist->tracks();
	if(!Util::between(row, tracks) || tracks[row].isDisabled())
	{
		return Qt::NoItemFlags;
	}

	const auto itemFlags = QAbstractTableModel::flags(index);
	return (index.column() == static_cast<int>(ColumnName::Description))
	       ? (itemFlags | Qt::ItemIsEditable)
	       : (itemFlags | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
}

void Model::clear()
{
	::Playlist::clear(*m->playlist, Reason::UserInterface);
}

void Model::removeTracks(const IndexSet& indexes)
{
	::Playlist::removeTracks(*m->playlist, indexes, Reason::UserInterface);
}

IndexSet Model::moveTracks(const IndexSet& indexes, int targetIndex)
{
	return ::Playlist::moveTracks(*m->playlist, indexes, targetIndex, Reason::UserInterface);
}

IndexSet Model::moveTracksUp(const IndexSet& indexes)
{
	const auto minRow = *(std::min_element(indexes.begin(), indexes.end()));
	return (minRow <= 0)
	       ? IndexSet()
	       : moveTracks(indexes, minRow - 1);
}

IndexSet Model::moveTracksDown(const IndexSet& indexes)
{
	auto minMax = std::minmax_element(indexes.begin(), indexes.end());
	int minRow = *(minMax.first);
	int maxRow = *(minMax.second);

	return (maxRow < rowCount() - 1)
	       ? moveTracks(indexes, minRow + static_cast<int>(indexes.size()) + 1)
	       : IndexSet();

}

IndexSet Model::copyTracks(const IndexSet& indexes, const int targetIndex)
{
	return ::Playlist::copyTracks(*m->playlist, indexes, targetIndex, Reason::UserInterface);
}

void Model::changeRating(const IndexSet& indexes, const Rating rating)
{
	const auto& playlistTracks = m->playlist->tracks();

	auto modifiedTracks = MetaDataList {};
	for(const auto idx: indexes)
	{
		if(Util::between(idx, playlistTracks))
		{
			auto track = playlistTracks[idx];
			if(rating != track.rating())
			{
				modifiedTracks << track;
			}
		}
	}

	if(!modifiedTracks.isEmpty())
	{
		if(!m->uto)
		{
			m->uto = new Tagging::UserOperations(Tagging::TagReader::create(), Tagging::TagWriter::create(), -1, this);
		}

		m->uto->setTrackRating(modifiedTracks, rating);
	}
}

void Model::insertTracks(const MetaDataList& tracks, const int row)
{
	::Playlist::insertTracks(*m->playlist, tracks, row, Reason::UserInterface);
}

void Model::insertTracks(const QStringList& files, const int row)
{
	auto* playlistGenerator = new LocalPathProcessor(m->playlist);
	connect(playlistGenerator, &LocalPathProcessor::sigFinished, playlistGenerator, &QObject::deleteLater);
	playlistGenerator->insertPaths(files, row);
}

void Model::reverseTracks()
{
	::Playlist::reverse(*m->playlist, Reason::UserInterface);
}

void Model::randomizeTracks()
{
	::Playlist::randomize(*m->playlist, Reason::UserInterface);
}

void Model::sortTracks(const Library::TrackSortorder sortOrder)
{
	::Playlist::sortTracks(*m->playlist, sortOrder, Reason::UserInterface);
}

void Playlist::Model::jumpToNextAlbum()
{
	::Playlist::jumpToNextAlbum(*m->playlist);
}

int Model::currentTrack() const
{
	return m->playlist->currentTrackIndex();
}

void Model::changeTrack(const int trackIndex, const Seconds seconds)
{
	m->playlist->changeTrack(trackIndex, seconds * 1000); // NOLINT(readability-magic-numbers)
}

MetaDataList Model::metadata(const IndexSet& rows) const
{
	MetaDataList tracks;

	const auto& playlistTracks = m->playlist->tracks();
	for(const auto row: rows)
	{
		if(Util::between(row, playlistTracks))
		{
			tracks << playlistTracks[row];
		}
	}

	return tracks;
}

QModelIndexList Model::searchResults(const QString& searchString)
{
	const auto [playlistSearchMode, cleanedSearchString] = evaluateSearchString(searchString);

	if(playlistSearchMode == PlaylistSearchMode::Jump)
	{
		const auto row = extractRowFromSearchstring(cleanedSearchString, rowCount() - 1);
		return (row >= 0)
		       ? QModelIndexList {index(row, 0)}
		       : QModelIndexList {QModelIndex {}};
	}

	const auto& tracks = m->playlist->tracks();
	for(auto i = 0; i < rowCount(); i++)
	{
		const auto& track = tracks[i];
		const auto searchKey = calculateSearchKey(track, playlistSearchMode, searchMode());

		if(searchKey.contains(cleanedSearchString))
		{
			return QModelIndexList {index(i, 0)};
		}
	}

	return QModelIndexList {};
}

using ExtraTriggerMap = SearchableModelInterface::ExtraTriggerMap;

ExtraTriggerMap Model::getExtraTriggers()
{
	ExtraTriggerMap map;

	map.insert(ArtistSearchPrefix, Lang::get(Lang::Artist));
	map.insert(AlbumSearchPrefix, Lang::get(Lang::Album));
	map.insert(FilenameSearchPrefix, Lang::get(Lang::Filename));
	map.insert(JumpPrefix, tr("Goto row"));

	return map;
}

QList<int> toSortedList(const QModelIndexList& indexes, const int maxRow)
{
	Util::Set<int> rowSet;
	for(const auto& index: indexes)
	{
		const auto row = index.row();
		if(Util::between(row, maxRow))
		{
			rowSet << index.row();
		}
	}

	auto rows = rowSet.toList();
	Algorithm::sort(rows, [](int row1, int row2) {
		return (row1 < row2);
	});

	return rows;
}

QMimeData* Model::mimeData(const QModelIndexList& indexes) const
{
	if(indexes.isEmpty())
	{
		return nullptr;
	}

	const auto& playlistTracks = m->playlist->tracks();
	const auto rows = toSortedList(indexes, playlistTracks.count());

	MetaDataList tracks;
	for(const auto row: rows)
	{
		tracks << playlistTracks[row];
	}

	if(tracks.empty())
	{
		return nullptr;
	}

	auto* mimedata = new Gui::CustomMimeData(this);
	mimedata->setMetadata(tracks);
	mimedata->setPlaylistSourceIndex(m->playlist->index());

	return mimedata;
}

bool Model::hasLocalMedia(const IndexSet& rows) const
{
	const auto& tracks = m->playlist->tracks();

	return Algorithm::contains(rows, [tracks](const auto row) {
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

void Model::refreshData()
{
	m->coverLookupMap.clear();
	::Playlist::enableAll(*m->playlist, Reason::UserInterface);
}

void Model::lookChanged()
{
	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void Model::refreshPlaylist(int rowCount, int columnCount)
{
	if(m->oldRowCount > rowCount)
	{
		beginRemoveRows(QModelIndex(), rowCount, m->oldRowCount - 1);
		endRemoveRows();
	}

	else if(rowCount > m->oldRowCount)
	{
		beginInsertRows(QModelIndex(), m->oldRowCount, rowCount - 1);
		endInsertRows();
	}

	if(rowCount == 0)
	{
		beginResetModel();
		endResetModel();
	}

	m->oldRowCount = rowCount;

	emit dataChanged(index(0, 0), index(rowCount - 1, columnCount - 1));
	emit sigDataReady();
}

void Model::playlistChanged([[maybe_unused]] int playlistIndex)
{
	if(playlistIndex == m->playlist->index())
	{
		refreshPlaylist(::Playlist::count(*m->playlist), columnCount());
	}
}

void Model::currentTrackChanged(int oldIndex, int newIndex)
{
	if(Util::between(oldIndex, ::Playlist::count(*m->playlist)))
	{
		emit dataChanged(index(oldIndex, 0), index(oldIndex, columnCount() - 1));
	}

	if(Util::between(newIndex, ::Playlist::count(*m->playlist)))
	{
		emit dataChanged(index(newIndex, 0), index(newIndex, columnCount() - 1));
		emit sigCurrentTrackChanged(newIndex);
	}
}

void Playlist::Model::deleteTracks(const IndexSet& rows)
{
	auto tracks = MetaDataList {};
	const auto& playlistTracks = m->playlist->tracks();
	for(const auto& row: rows)
	{
		if(Util::between(row, playlistTracks))
		{
			tracks << playlistTracks[row];
		}
	}

	::Playlist::removeTracks(*m->playlist, rows, Reason::TracksDeleted);
	m->libraryInteractor->deleteTracks(tracks);
}

void Playlist::Model::findTrack(const int index)
{
	const auto& tracks = m->playlist->tracks();
	if(Util::between(index, tracks))
	{
		m->libraryInteractor->findTrack(tracks[index]);
	}
}

int Playlist::Model::playlistIndex() const
{
	return m->playlist->index();
}

void Playlist::Model::setBusy(bool b)
{
	m->playlist->setBusy(b);
}

void Playlist::Model::coverFound(const QPixmap& pixmap)
{
	if(auto* coverLookup = dynamic_cast<Cover::Lookup*>(sender()); coverLookup)
	{
		const auto hash = coverLookup->userData<QString>();
		m->coverLookupMap[hash] = pixmap;

		const auto& tracks = m->playlist->tracks();

		auto row = 0;
		for(const auto& track: tracks)
		{
			const auto trackHash = getAlbumHashFromTrack(track);
			if(trackHash == hash)
			{
				constexpr const auto column = static_cast<int>(ColumnName::Cover);
				emit dataChanged(index(row, column), index(row, column));
			}

			row++;
		}
	}
}

void Playlist::Model::coversChanged()
{
	m->coverLookupMap.clear();

	constexpr const auto column = static_cast<int>(ColumnName::Cover);
	emit dataChanged(index(0, column), index(rowCount() - 1, column));
}

void Playlist::Model::startCoverLookup(const MetaData& track) const
{
	const auto coverLocation = Cover::Location::coverLocation(track);
	auto* coverLookup = new Cover::Lookup(coverLocation, 1, nullptr);
	coverLookup->setUserData(getAlbumHashFromTrack(track));
	connect(coverLookup, &Cover::Lookup::sigCoverFound, this, &Model::coverFound);
	connect(coverLookup, &Cover::Lookup::sigFinished, this, &Model::coverLookupFinished);
	coverLookup->start();
}

void Playlist::Model::coverLookupFinished([[maybe_unused]] bool success)
{
	if(sender())
	{
		sender()->deleteLater();
	}
}

bool Playlist::Model::isLocked() const { return m->playlist->isLocked(); }

void Playlist::Model::setLocked(const bool b) { m->playlist->setLocked(b); }
