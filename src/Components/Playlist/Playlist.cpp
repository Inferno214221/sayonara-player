/* Playlist.cpp */

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

#include "Playlist.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Set.h"
#include "Utils/FileUtils.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Interfaces/PlayManager.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/Logger/Logger.h"

#include <QMultiHash>

namespace File = Util::File;
namespace Algorithm = Util::Algorithm;

using PlaylistImpl = ::Playlist::Playlist;

struct PlaylistImpl::Private
{
	PlayManager* playManager;
	MetaData dummyTrack;
	MetaDataList tracks;
	Tagging::ChangeNotifier* metadataChangeNotifier;
	QList<UniqueId> shuffleHistory;
	UniqueId playingUniqueId;
	PlaylistMode playlistMode;
	int playlistIndex;
	bool playlistChanged;
	bool busy;

	Private(int playlist_idx, PlaylistMode playlist_mode, PlayManager* playManager) :
		playManager(playManager),
		metadataChangeNotifier(Tagging::ChangeNotifier::instance()),
		playlistMode(playlist_mode),
		playlistIndex(playlist_idx),
		playlistChanged(false),
		busy(false) {}
};

PlaylistImpl::Playlist(int idx, const QString& name, PlayManager* playManager) :
	Playlist::DBInterface(name)
{
	m = Pimpl::make<::Playlist::Playlist::Private>(idx, GetSetting(Set::PL_Mode), playManager);

	connect(m->metadataChangeNotifier, &Tagging::ChangeNotifier::sigMetadataChanged, this, &Playlist::metadataChanged);
	connect(m->metadataChangeNotifier, &Tagging::ChangeNotifier::sigMetadataDeleted, this, &Playlist::metadataDeleted);

	connect(m->playManager, &PlayManager::sigCurrentMetadataChanged, this, &Playlist::currentMetadataChanged);
	connect(m->playManager, &PlayManager::sigDurationChangedMs, this, &Playlist::durationChanged);

	ListenSetting(Set::PL_Mode, Playlist::settingPlaylistModeChanged);
}

PlaylistImpl::~Playlist() = default;

void PlaylistImpl::clear()
{
	if(!m->tracks.isEmpty())
	{
		m->tracks.clear();
		setChanged(true);
	}
}

IndexSet PlaylistImpl::moveTracks(const IndexSet& indexes, int targetRow)
{
	m->tracks.moveTracks(indexes, targetRow);

	const auto lineCountBeforeTarget = Algorithm::count(indexes, [&targetRow](const auto index) {
		return (index < targetRow);
	});

	IndexSet newTrackPositions;
	for(auto i = targetRow; i < targetRow + indexes.count(); i++)
	{
		newTrackPositions.insert(i - lineCountBeforeTarget);
	}

	setChanged(true);

	return newTrackPositions;
}

IndexSet PlaylistImpl::copyTracks(const IndexSet& indexes, int tgt)
{
	m->tracks.copyTracks(indexes, tgt);

	setChanged(true);

	IndexSet newTrackPositions;
	for(auto i = 0; i < indexes.count(); i++)
	{
		newTrackPositions << tgt + i;
	}

	setChanged(true);

	return newTrackPositions;
}

void Playlist::Playlist::findTrack(int idx)
{
	if(Util::between(idx, m->tracks))
	{
		emit sigFindTrackRequested(m->tracks[idx]);
	}
}

void PlaylistImpl::removeTracks(const IndexSet& indexes)
{
	m->tracks.removeTracks(indexes);
	setChanged(true);
}

void PlaylistImpl::insertTracks(const MetaDataList& tracks, int targetIndex)
{
	m->tracks.insertTracks(tracks, targetIndex);
	setChanged(true);
}

void PlaylistImpl::appendTracks(const MetaDataList& tracks)
{
	if(isBusy())
	{
		return;
	}

	const auto oldTrackCount = m->tracks.count();

	m->tracks.append(tracks);

	for(auto it = m->tracks.begin() + oldTrackCount; it != m->tracks.end(); it++)
	{
		const auto fileExists = File::checkFile(it->filepath());
		it->setDisabled(!fileExists);
	}

	setChanged(true);
}

bool PlaylistImpl::changeTrack(int index, MilliSeconds positionMs)
{
	const auto oldIndex = this->currentTrackIndex();
	setTrackIndexBeforeStop(-1);
	setCurrentTrack(index);

	if(!Util::between(index, m->tracks))
	{
		stop();
		setTrackIndexBeforeStop(-1);
		return false;
	}

	m->shuffleHistory << m->tracks[index].uniqueId();

	if(!Util::File::checkFile(m->tracks[index].filepath()))
	{
		spLog(Log::Warning, this)
			<< QString("Track %1 not available on file system: ").arg(m->tracks[index].filepath());
		m->tracks[index].setDisabled(true);

		return changeTrack(index + 1);
	}

	m->playManager->play();
	if(positionMs > 0)
	{
		m->playManager->seekAbsoluteMs(positionMs);
	}

	emit sigTrackChanged(oldIndex, this->currentTrackIndex());

	return true;
}

void PlaylistImpl::metadataDeleted()
{
	IndexSet indexes;
	const auto deletedTracks = m->metadataChangeNotifier->deletedMetadata();

	auto it = std::remove_if(m->tracks.begin(), m->tracks.end(), [deletedTracks](const auto& track) {
		return Algorithm::contains(deletedTracks, [&track](const auto& tmpTrack) {
			return (track.isEqual(tmpTrack));
		});
	});

	m->tracks.erase(it, m->tracks.end());
	emit sigItemsChanged(index());
}

void PlaylistImpl::metadataChanged()
{
	const auto& changedTracks = m->metadataChangeNotifier->changedMetadata();

	auto filepathIndexMap = QMultiHash<QString, int>();

	auto i = 0;
	for(auto it = m->tracks.begin(); it != m->tracks.end(); it++, i++)
	{
		filepathIndexMap.insert(it->filepath(), i);
	}

	for(const auto& changedPair : changedTracks)
	{
		const auto filepath = changedPair.first.filepath();
		const auto indexes = filepathIndexMap.values(filepath);
		for(const auto& index : indexes)
		{
			replaceTrack(index, changedPair.second);
		}
	}

	emit sigItemsChanged(this->index());
}

void PlaylistImpl::currentMetadataChanged()
{
	const auto track = m->playManager->currentTrack();
	const auto indexList = m->tracks.findTracks(track.filepath());

	for(const auto index : indexList)
	{
		replaceTrack(index, track);
	}
}

void PlaylistImpl::durationChanged()
{
	const auto currentTrack = m->playManager->currentTrack();
	const auto indexList = m->tracks.findTracks(currentTrack.filepath());

	for(const auto index : indexList)
	{
		auto track = (m->tracks[index]);
		track.setDurationMs(std::max<MilliSeconds>(0, currentTrack.durationMs()));
		replaceTrack(index, track);
	}
}

void PlaylistImpl::replaceTrack(int index, const MetaData& track)
{
	if(!Util::between(index, m->tracks))
	{
		return;
	}

	const auto oldUniqueId = m->tracks[index].uniqueId();
	const auto isCurrent = (oldUniqueId == m->playingUniqueId);
	const auto fileExists = File::checkFile(track.filepath());

	m->tracks[index] = track;
	m->tracks[index].setDisabled(!fileExists);

	if(isCurrent)
	{
		m->playingUniqueId = m->tracks[index].uniqueId();
	}

	auto i = 0;
	for(const auto& uniqueId : m->shuffleHistory)
	{
		if(uniqueId == oldUniqueId)
		{
			m->shuffleHistory.replace(i, m->tracks[index].uniqueId());
		}

		i++;
	}

	emit sigItemsChanged(this->index());
}

void PlaylistImpl::play()
{
	if(currentTrackIndex() < 0)
	{
		changeTrack(0);
	}
}

void PlaylistImpl::stop()
{
	m->shuffleHistory.clear();
	const auto currentTrack = currentTrackIndex();

	if(currentTrack >= 0)
	{
		setTrackIndexBeforeStop(currentTrack);
		setCurrentTrack(-1);
	}

	emit sigTrackChanged(currentTrack, -1);
	emit sigStopped();
}

void PlaylistImpl::fwd()
{
	auto currentMode = m->playlistMode;
	const auto oldMode = m->playlistMode;

	// temp. disable rep1 as we want a new track
	currentMode.setRep1(false);
	setMode(currentMode);

	next();

	setMode(oldMode);
}

void PlaylistImpl::bwd()
{
	if(m->playManager->currentPositionMs() > 2000)
	{
		m->playManager->seekAbsoluteMs(0);
		return;
	}

	if(PlaylistMode::isActiveAndEnabled(m->playlistMode.shuffle()))
	{
		for(auto historyIndex = m->shuffleHistory.size() - 2; historyIndex >= 0; historyIndex--)
		{
			const auto uniqueId = m->shuffleHistory[historyIndex];
			const auto index = Util::Algorithm::indexOf(m->tracks, [uniqueId](const auto& track) {
				return (uniqueId == track.uniqueId());
			});

			if(index >= 0)
			{
				m->shuffleHistory.erase(m->shuffleHistory.begin() + historyIndex, m->shuffleHistory.end());

				changeTrack(index);
				return;
			}
		}
	}

	m->shuffleHistory.clear();
	changeTrack(currentTrackIndex() - 1);
}

void PlaylistImpl::next()
{
	// no track
	if(m->tracks.isEmpty())
	{
		stop();
		setTrackIndexBeforeStop(-1);
		return;
	}

	const auto rep1 = PlaylistMode::isActiveAndEnabled(m->playlistMode.rep1());
	const auto shuffle = PlaylistMode::isActiveAndEnabled(m->playlistMode.shuffle());
	const auto repAll = (PlaylistMode::isActiveAndEnabled(m->playlistMode.repAll()));
	const auto isLastTrack = (currentTrackIndex() == m->tracks.count() - 1);

	// stopped
	auto trackIndex = -1;

	if(currentTrackIndex() == -1)
	{
		trackIndex = 0;
	}

	else if(rep1)
	{
		trackIndex = currentTrackIndex();
	}

	else if(shuffle)
	{
		trackIndex = calcShuffleTrack();
	}

	else
	{// normal track
		// last track
		if(isLastTrack)
		{
			trackIndex = repAll ? 0 : -1;
		}

		else
		{
			trackIndex = currentTrackIndex() + 1;
		}
	}

	changeTrack(trackIndex);
}

int PlaylistImpl::calcShuffleTrack()
{
	if(m->tracks.count() <= 1)
	{
		return -1;
	}

	// check all tracks played
	auto i = 0;
	auto unplayedTracks = QList<int>();
	for(const auto& track : m->tracks)
	{
		const auto uniqueId = track.uniqueId();
		if(!m->shuffleHistory.contains(uniqueId))
		{
			unplayedTracks << i;
		}

		i++;
	}

	// no random track to play
	if(unplayedTracks.isEmpty())
	{
		if(PlaylistMode::isActiveAndEnabled(m->playlistMode.repAll()) == false)
		{
			return -1;
		}

		m->shuffleHistory.clear();
		return Util::randomNumber(0, int(m->tracks.size() - 1));
	}

	else
	{
		const auto randomIndex = Util::randomNumber(0, unplayedTracks.size() - 1);
		return unplayedTracks[randomIndex];
	}
}

bool PlaylistImpl::wakeUp()
{
	const auto index = trackIndexBeforeStop();

	return (Util::between(index, count()))
	       ? changeTrack(index)
	       : false;
}

void Playlist::Playlist::setBusy(bool busy)
{
	m->busy = busy;

	emit sigBusyChanged(busy);
}

bool Playlist::Playlist::isBusy() const
{
	return m->busy;
}

void Playlist::Playlist::reverse()
{
	std::reverse(m->tracks.begin(), m->tracks.end());
	setChanged(true);
}

void PlaylistImpl::enableAll()
{
	for(auto& track : m->tracks)
	{
		track.setDisabled(false);
	}

	setChanged(true);
}

int PlaylistImpl::createPlaylist(const MetaDataList& tracks)
{
	const auto append = PlaylistMode::isActiveAndEnabled(m->playlistMode.append());
	if(!append)
	{
		m->tracks.clear();
		m->shuffleHistory.clear();
	}

	m->tracks << tracks;

	setChanged(true);

	return m->tracks.count();
}

int PlaylistImpl::index() const
{
	return m->playlistIndex;
}

void PlaylistImpl::setIndex(int idx)
{
	m->playlistIndex = idx;
}

void PlaylistImpl::setMode(const PlaylistMode& mode)
{
	if(m->playlistMode.shuffle() != mode.shuffle())
	{
		m->shuffleHistory.clear();
	}

	m->playlistMode = mode;
}

PlaylistMode PlaylistImpl::mode() const
{
	return m->playlistMode;
}

MilliSeconds PlaylistImpl::runningTime() const
{
	const auto durationMs =
		std::accumulate(m->tracks.begin(),
		                m->tracks.end(),
		                0,
		                [](const auto timeMs, const auto& track) {
			                return timeMs + track.durationMs();
		                });

	return durationMs;
}

int PlaylistImpl::currentTrackIndex() const
{
	if(m->playingUniqueId == 0)
	{
		return -1;
	}

	return Util::Algorithm::indexOf(m->tracks, [&](const auto& track) {
		return (track.uniqueId() == m->playingUniqueId);
	});
}

bool PlaylistImpl::currentTrack(MetaData& track) const
{
	const auto trackIndex = currentTrackIndex();
	if(!Util::between(trackIndex, m->tracks))
	{
		return false;
	}

	track = m->tracks[trackIndex];
	return true;
}

void Playlist::Playlist::setCurrentTrack(int index)
{
	if(!Util::between(index, m->tracks))
	{
		m->playingUniqueId = 0;
		stop();
	}

	else
	{
		const auto& track = m->tracks[index];
		m->playingUniqueId = track.uniqueId();
		m->playManager->changeCurrentTrack(track, index);
		SetSetting(Set::PL_LastPlaylist, this->id());
	}
}

int PlaylistImpl::count() const
{
	return m->tracks.count();
}

void PlaylistImpl::setChanged(bool b)
{
	restoreTrackBeforeStop();

	m->playlistChanged = b;

	emit sigItemsChanged(m->playlistIndex);
}

bool PlaylistImpl::wasChanged() const
{
	return m->playlistChanged;
}

void PlaylistImpl::settingPlaylistModeChanged()
{
	setMode(GetSetting(Set::PL_Mode));
}

const MetaDataList& PlaylistImpl::tracks() const
{
	return m->tracks;
}

const MetaData& PlaylistImpl::track(int index) const
{
	return (Util::between(index, m->tracks))
	       ? m->tracks[index]
	       : m->dummyTrack;
}

void PlaylistImpl::reloadFromDatabase()
{
	if(!this->isBusy())
	{
		const auto customPlaylist = this->fetchTracksFromDatabase();
		this->clear();
		this->createPlaylist(customPlaylist);
	}
}

void Playlist::Playlist::deleteTracks(const IndexSet& indexes)
{
	MetaDataList tracks;
	for(const auto& index : indexes)
	{
		if(Util::between(index, m->tracks))
		{
			tracks << m->tracks[index];
		}
	}

	emit sigDeleteFilesRequested(tracks);
}
