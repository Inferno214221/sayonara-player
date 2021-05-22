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
#include "Components/Playlist/ExternTracksPlaylistGenerator.h"
#include "Components/Tagging/UserTaggingOperations.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Covers/CoverLookup.h"

#include "Interfaces/PlaylistInterface.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Gui/Utils/Icons.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Set.h"
#include "Utils/globals.h"

#include "Utils/Language/Language.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"

#include <QFont>
#include <QUrl>
#include <QHash>
#include <QIcon>
#include <QPixmap>

namespace Algorithm = Util::Algorithm;
using Playlist::Model;

namespace
{
	constexpr const auto AlbumSearchPrefix = '%';
	constexpr const auto ArtistSearchPrefix = '$';
	constexpr const auto JumpPrefix = ':';

	static QString convertEntryLook(const QString& entryLook, const MetaData& md)
	{
		auto ret = entryLook;
		ret.replace(QStringLiteral("*"), QChar(Model::Bold));
		ret.replace(QStringLiteral("'"), QChar(Model::Italic));

		ret.replace(QStringLiteral("%title%"), md.title());
		ret.replace(QStringLiteral("%nr%"), QString::number(md.trackNumber()));
		ret.replace(QStringLiteral("%artist%"), md.artist());
		ret.replace(QStringLiteral("%album%"), md.album());

		return ret;
	}
}

enum class PlaylistSearchMode
{
		Artist,
		Album,
		Title,
		Jump
};

struct Model::Private
{
	QHash<AlbumId, QPixmap> coverLookupMap;
	int oldRowCount;
	int dragIndex;
	int rowHeight;
	PlaylistPtr playlist;
	Tagging::UserOperations* uto = nullptr;
	PlaylistCreator* playlistCreator;

	Private(PlaylistCreator* playlistCreator, PlaylistPtr playlistArg) :
		oldRowCount(0),
		dragIndex(-1),
		rowHeight(20),
		playlist(playlistArg),
		playlistCreator {playlistCreator} {}
};

Model::Model(PlaylistCreator* playlistCreator, PlaylistPtr playlist, QObject* parent) :
	SearchableTableModel(parent)
{
	m = Pimpl::make<Private>(playlistCreator, playlist);

	connect(m->playlist.get(), &Playlist::Playlist::sigItemsChanged, this, &Model::playlistChanged);
	connect(m->playlist.get(), &Playlist::Playlist::sigTrackChanged, this, &Model::currentTrackChanged);

	const auto coverChangeNotifier = Cover::ChangeNotfier::instance();
	connect(coverChangeNotifier, &Cover::ChangeNotfier::sigCoversChanged, this, &Model::coversChanged);

	ListenSettingNoCall(Set::PL_EntryLook, Model::lookChanged);

	playlistChanged(0);
}

Model::~Model() = default;

int Model::rowCount([[maybe_unused]] const QModelIndex& parent) const
{
	return m->playlist->count();
}

int Model::columnCount([[maybe_unused]] const QModelIndex& parent) const
{
	return int(ColumnName::NumColumns);
}

QVariant Model::data(const QModelIndex& index, int role) const
{
	const auto row = index.row();
	const auto col = index.column();
	const auto isCurrentTrack = (row == m->playlist->currentTrackIndex());

	if(!Util::between(row, m->playlist->count()))
	{
		return QVariant();
	}

	if(role == Qt::DisplayRole)
	{
		if(col == ColumnName::TrackNumber)
		{
			return (isCurrentTrack)
			       ? QString()
			       : QString("%1.").arg(row + 1);
		}

		else if(col == ColumnName::Time)
		{
			auto durationMs = m->playlist->track(row).durationMs();
			return (durationMs / 1000 <= 0)
			       ? QVariant()
			       : Util::msToString(durationMs, QStringLiteral("$M:$S"));
		}

		return QVariant();
	}

	else if(role == Qt::TextAlignmentRole)
	{
		if(col != ColumnName::Description)
		{
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		}
	}

	else if(role == Qt::DecorationRole)
	{
		if(col == ColumnName::Cover)
		{
			const auto& track = m->playlist->track(row);
			if(!m->coverLookupMap.contains(track.albumId()))
			{
				m->coverLookupMap.insert(track.albumId(), QPixmap {});

				const auto coverLocation = Cover::Location::coverLocation(track);
				auto* coverLookup = new Cover::Lookup(coverLocation, 1, nullptr);
				coverLookup->setUserData(track.albumId());
				connect(coverLookup, &Cover::Lookup::sigCoverFound, this, &Model::coverFound);
				connect(coverLookup, &Cover::Lookup::sigFinished, this, &Model::coverLookupFinished);
				coverLookup->start();
			}

			return QIcon(m->coverLookupMap[track.albumId()]);
		}
	}

	else if(role == Qt::SizeHintRole)
	{
		if(col == ColumnName::Cover)
		{
			const auto h = m->rowHeight - 4;
			return QSize(h, h);
		}
	}

	else if(role == Model::EntryLookRole)
	{
		if(col == ColumnName::Description)
		{
			return convertEntryLook(GetSetting(Set::PL_EntryLook), m->playlist->track(row));
		}
	}

	else if(role == Model::DragIndexRole)
	{
		return (row == m->dragIndex);
	}

	else if(role == Model::RatingRole || role == Qt::EditRole)
	{
		if(col == ColumnName::Description)
		{
			const auto& track = m->playlist->track(row);
			return (track.radioMode() == RadioMode::Off)
			       ? QVariant::fromValue(metadata(row).rating())
			       : QVariant::fromValue(Rating::Last);
		}
	}

	else if(role == Model::CurrentPlayingRole)
	{
		return isCurrentTrack;
	}

	return QVariant();
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
	if(!Util::between(row, m->playlist->count()) || metadata(row).isDisabled())
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
	m->playlist->clear();
}

void Model::removeTracks(const IndexSet& indexes)
{
	m->playlist->removeTracks(indexes);
}

IndexSet Model::moveTracks(const IndexSet& indexes, int target_index)
{
	return m->playlist->moveTracks(indexes, target_index);
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

IndexSet Model::copyTracks(const IndexSet& indexes, int target_index)
{
	return m->playlist->copyTracks(indexes, target_index);
}

void Model::changeRating(const IndexSet& indexes, Rating rating)
{
	MetaDataList tracks;
	tracks.reserve(indexes.size());

	for(auto idx : indexes)
	{
		auto track = m->playlist->track(idx);
		if(rating != track.rating())
		{
			tracks << track;
			track.setRating(rating);

			m->playlist->replaceTrack(idx, track);
		}

		emit dataChanged(index(idx, 0), index(idx, int(ColumnName::Description)));
	}

	if(tracks.isEmpty())
	{
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
	m->playlist->insertTracks(tracks, row);
}

void Model::insertTracks(const QStringList& files, int row)
{
	auto* playlistGenerator = new ExternTracksPlaylistGenerator(m->playlistCreator, m->playlist);
	connect(playlistGenerator, &ExternTracksPlaylistGenerator::sigFinished, playlistGenerator, &QObject::deleteLater);
	playlistGenerator->insertPaths(files, row);
}

void Model::reverseTracks()
{
	m->playlist->reverse();
}

int Model::currentTrack() const
{
	return m->playlist->currentTrackIndex();
}

const MetaData& Model::metadata(int row) const
{
	return m->playlist->track(row);
}

MetaDataList Model::metadata(const IndexSet& rows) const
{
	MetaDataList tracks;
	tracks.reserve(rows.size());

	Util::Algorithm::transform(rows, tracks, [this](int row) {
		return m->playlist->track(row);
	});

	return tracks;
}

QModelIndexList Model::searchResults(const QString& substr)
{
	QModelIndexList ret;
	auto pureSearchString = substr;
	auto playlistSearchMode = PlaylistSearchMode::Title;

	if(pureSearchString.startsWith(ArtistSearchPrefix))
	{
		playlistSearchMode = PlaylistSearchMode::Artist;
		pureSearchString.remove(ArtistSearchPrefix);
	}
	else if(pureSearchString.startsWith(AlbumSearchPrefix))
	{
		playlistSearchMode = PlaylistSearchMode::Album;
		pureSearchString.remove(AlbumSearchPrefix);
	}
	else if(pureSearchString.startsWith(JumpPrefix))
	{
		playlistSearchMode = PlaylistSearchMode::Jump;
		pureSearchString.remove(JumpPrefix);
	}

	pureSearchString = pureSearchString.trimmed();

	if(playlistSearchMode == PlaylistSearchMode::Jump)
	{
		bool ok;
		const auto line = pureSearchString.toInt(&ok);

		return (ok && line < rowCount())
		       ? QModelIndexList {this->index(line, 0)}
		       : QModelIndexList {QModelIndex {}};
	}

	const auto rows = rowCount();
	for(int i = 0; i < rows; i++)
	{
		const auto& track = m->playlist->track(i);
		QString str;
		switch(playlistSearchMode)
		{
			case PlaylistSearchMode::Artist:
				str = track.artist();
				break;
			case PlaylistSearchMode::Album:
				str = track.album();
				break;
			default:
				str = track.title();
				break;
		}

		str = Library::Utils::convertSearchstring(str, searchMode());
		if(str.contains(pureSearchString))
		{
			return QModelIndexList {this->index(i, 0)};
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
	map.insert(JumpPrefix, tr("Goto row"));

	return map;
}

QMimeData* Model::mimeData(const QModelIndexList& indexes) const
{
	if(indexes.isEmpty())
	{
		return nullptr;
	}

	Util::Set<int> rowSet;
	for(const auto& index : indexes)
	{
		rowSet << index.row();
	}

	auto rows = rowSet.toList();
	Algorithm::sort(rows, [](int row1, int row2) {
		return (row1 < row2);
	});

	MetaDataList tracks;
	tracks.reserve(static_cast<MetaDataList::size_type>(rows.size()));

	for(const auto row : Algorithm::AsConst(rows))
	{
		if(row < m->playlist->count())
		{
			tracks << m->playlist->track(row);
		}
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
	m->coverLookupMap.clear();
	m->playlist->enableAll();
}

void Model::lookChanged()
{
	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void Model::playlistChanged([[maybe_unused]] int playlistIndex)
{
	if(m->oldRowCount > m->playlist->count())
	{
		beginRemoveRows(QModelIndex(), m->playlist->count(), m->oldRowCount - 1);
		endRemoveRows();
	}

	else if(m->playlist->count() > m->oldRowCount)
	{
		beginInsertRows(QModelIndex(), m->oldRowCount, m->playlist->count() - 1);
		endInsertRows();
	}

	if(m->playlist->count() == 0)
	{
		beginResetModel();
		endResetModel();
	}

	m->oldRowCount = m->playlist->count();

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
	emit sigDataReady();
}

void Model::currentTrackChanged(int oldIndex, int newIndex)
{
	if(Util::between(oldIndex, m->playlist->count()))
	{
		emit dataChanged(index(oldIndex, 0), index(oldIndex, columnCount() - 1));
	}

	if(Util::between(oldIndex, m->playlist->count()))
	{
		emit dataChanged(index(newIndex, 0), index(newIndex, columnCount() - 1));
		emit sigCurrentTrackChanged(newIndex);
	}
}

void Playlist::Model::deleteTracks(const IndexSet& rows)
{
	m->playlist->deleteTracks(rows);
}

void Playlist::Model::findTrack(int index)
{
	m->playlist->findTrack(index);
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
	if(auto* coverLookup = static_cast<Cover::Lookup*>(sender()); coverLookup)
	{
		const auto albumId = coverLookup->userData<AlbumId>();
		m->coverLookupMap[albumId] = pixmap;

		const auto& tracks = m->playlist->tracks();

		auto row = 0;
		for(const auto& track : tracks)
		{
			if(track.albumId() == albumId)
			{
				constexpr const auto column = static_cast<int>(ColumnName::Cover);
				emit dataChanged(index(row, column), index(row, column));
			}

			row++;
		}
	}
}

void Playlist::Model::coverLookupFinished([[maybe_unused]] bool success)
{
	if(sender())
	{
		sender()->deleteLater();
	}
}

void Playlist::Model::coversChanged()
{
	m->coverLookupMap.clear();

	constexpr const auto column = static_cast<int>(ColumnName::Cover);
	emit dataChanged(index(0, column), index(rowCount() - 1, column));
}
