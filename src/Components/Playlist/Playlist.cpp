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

#include "Components/PlayManager/PlayManager.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/Logger/Logger.h"

namespace File=Util::File;
namespace Algorithm=Util::Algorithm;

using PlaylistImpl=::Playlist::Playlist;

struct PlaylistImpl::Private
{
	MetaData		dummyTrack;
	MetaDataList    tracks;
	QList<UniqueId>	shuffleHistory;
	UniqueId		playingUniqueId;
	PlaylistMode	playlistMode;
	int				playlistIndex;
	PlaylistType	type;
	bool			playlistChanged;
	bool			busy;

	Private(int playlist_idx, PlaylistMode playlist_mode, PlaylistType type) :
		playlistMode(playlist_mode),
		playlistIndex(playlist_idx),
		type(type),
		playlistChanged(false),
		busy(false)
	{}
};


PlaylistImpl::Playlist(int idx, PlaylistType type, const QString& name) :
	Playlist::DBInterface(name)
{
	m = Pimpl::make<::Playlist::Playlist::Private>(idx,  GetSetting(Set::PL_Mode), type);

	auto* changeNotifier = Tagging::ChangeNotifier::instance();
	connect(changeNotifier, &Tagging::ChangeNotifier::sigMetadataChanged, this, &Playlist::metadataChanged);
	connect(changeNotifier, &Tagging::ChangeNotifier::sigMetadataDeleted, this, &Playlist::metadataDeleted);

	auto* playManager = PlayManager::instance();
	connect(playManager, &PlayManager::sigCurrentMetadataChanged, this, &Playlist::currentMetadataChanged);
	connect(playManager, &PlayManager::sigDurationChangedMs, this, &Playlist::durationChanged);

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

IndexSet PlaylistImpl::moveTracks(const IndexSet& indexes, int tgt_row)
{
	m->tracks.moveTracks(indexes, tgt_row);

	int lineCountBeforeTarget = Algorithm::count(indexes, [&tgt_row](int sel){
		return (sel < tgt_row);
	});

	IndexSet newTrackPositions;
	for(int i = tgt_row; i < tgt_row + indexes.count(); i++) {
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
	for(int i=0; i<indexes.count(); i++)
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
		emit sigFindTrack(m->tracks[idx].id());
	}
}

void PlaylistImpl::removeTracks(const IndexSet& indexes)
{
	m->tracks.removeTracks(indexes);
	setChanged(true);
}


void PlaylistImpl::insertTracks(const MetaDataList& lst, int tgt)
{
	m->tracks.insertTracks(lst, tgt);
	setChanged(true);
}

void PlaylistImpl::appendTracks(const MetaDataList& lst)
{
	int old_size = m->tracks.count();

	m->tracks.append(lst);

	for(auto it=m->tracks.begin() + old_size; it != m->tracks.end(); it++)
	{
		it->setDisabled
		(
			!(File::checkFile(it->filepath()))
		);
	}

	setChanged(true);
}

bool PlaylistImpl::changeTrack(int idx)
{
	setTrackIndexBeforeStop(-1);
	setCurrentTrack(idx);

	if( !Util::between(idx, m->tracks) )
	{
		stop();
		setTrackIndexBeforeStop(-1);
		return false;
	}

	m->shuffleHistory << m->tracks[idx].uniqueId();

	if( !Util::File::checkFile(m->tracks[idx].filepath()) )
	{
		spLog(Log::Warning, this) << QString("Track %1 not available on file system: ").arg(m->tracks[idx].filepath());
		m->tracks[idx].setDisabled(true);

		return changeTrack(idx + 1);
	}

	return true;
}

void PlaylistImpl::metadataDeleted()
{
	IndexSet indexes;
	auto* mdcn = Tagging::ChangeNotifier::instance();
	MetaDataList deletedTracks = mdcn->deletedMetadata();

	auto it = std::remove_if(m->tracks.begin(), m->tracks.end(), [deletedTracks](const MetaData& md)
	{
		return Algorithm::contains(deletedTracks, [&md](const MetaData& md_tmp){
			return (md.isEqual(md_tmp));
		});
	});

	m->tracks.erase(it, m->tracks.end());
	emit sigItemsChanged( index() );
}

void PlaylistImpl::metadataChanged()
{
	auto* mdcn = Tagging::ChangeNotifier::instance();
	auto changedTracks = mdcn->changedMetadata();

	const MetaDataList& changed_tracks = changedTracks.second;

	int i=0;
	for(auto it=m->tracks.begin(); it != m->tracks.end(); it++, i++)
	{
		auto it_changed = Algorithm::find(changed_tracks, [it](const MetaData& md)
		{
			return it->isEqual(md);
		});

		if(it_changed == changed_tracks.end()) {
			continue;
		}

		replaceTrack(i, *it_changed);
	}

	emit sigItemsChanged( index() );
}

void PlaylistImpl::currentMetadataChanged()
{
	const MetaData md = PlayManager::instance()->currentTrack();
	const IdxList idx_list = m->tracks.findTracks(md.filepath());

	for(int i : idx_list)
	{
		replaceTrack(i, md);
	}
}

void PlaylistImpl::durationChanged()
{
	MetaData current_track = PlayManager::instance()->currentTrack();
	IdxList idx_list = m->tracks.findTracks(current_track.filepath());

	for(int i : idx_list)
	{
		MetaData md(m->tracks[i]);
		md.setDurationMs(std::max<MilliSeconds>(0, current_track.durationMs()));
		replaceTrack(i, md);
	}
}


void PlaylistImpl::replaceTrack(int idx, const MetaData& md)
{
	if( !Util::between(idx, m->tracks) ) {
		return;
	}

	bool is_current = (m->tracks[idx].uniqueId() == m->playingUniqueId);
	m->tracks[idx] = md;
	m->tracks[idx].setDisabled
	(
		!(File::checkFile(md.filepath()))
	);

	if(is_current){
		m->playingUniqueId = m->tracks[idx].uniqueId();
	}

	emit sigItemsChanged(index());
}

void PlaylistImpl::play()
{
	if(currentTrackIndex() < 0){
		changeTrack(0);
	}
}

void PlaylistImpl::stop()
{
	m->shuffleHistory.clear();

	if(currentTrackIndex() >= 0)
	{
		setTrackIndexBeforeStop(currentTrackIndex());
		setCurrentTrack(-1);
	}

	emit sigStopped();
}

void PlaylistImpl::fwd()
{
	PlaylistMode cur_mode = m->playlistMode;
	PlaylistMode mode_bak = m->playlistMode;

	// temp. disable rep1 as we want a new track
	cur_mode.setRep1(false);
	setMode(cur_mode);

	next();

	setMode(mode_bak);
}

void PlaylistImpl::bwd()
{
	if(PlaylistMode::isActiveAndEnabled(m->playlistMode.shuffle()))
	{
		for(int history_index = m->shuffleHistory.size() - 2; history_index >= 0; history_index--)
		{
			UniqueId uid = m->shuffleHistory[history_index];

			int idx = Util::Algorithm::indexOf(m->tracks, [uid](const MetaData& md){
				return (uid == md.uniqueId());
			});

			if(idx >= 0)
			{
				m->shuffleHistory.erase(m->shuffleHistory.begin() + history_index, m->shuffleHistory.end());

				changeTrack(idx);
				return;
			}
		}
	}

	m->shuffleHistory.clear();
	changeTrack( currentTrackIndex() - 1 );
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

	// stopped
	int cur_track = currentTrackIndex();
	int track_num = -1;

	if(cur_track == -1){
		track_num = 0;
	}

	// play it again
	else if(PlaylistMode::isActiveAndEnabled(m->playlistMode.rep1())) {
		track_num = cur_track;
	}

	// shuffle mode
	else if(PlaylistMode::isActiveAndEnabled(m->playlistMode.shuffle())) {
		track_num = calcShuffleTrack();
	}

	// normal track
	else
	{
		// last track
		if(cur_track == m->tracks.count() - 1)
		{
			track_num = -1;

			if(PlaylistMode::isActiveAndEnabled(m->playlistMode.repAll())) {
				track_num = 0;
			}
		}

		else {
			track_num = cur_track + 1;
		}
	}

	changeTrack(track_num);
}


int PlaylistImpl::calcShuffleTrack()
{
	if(m->tracks.size() <= 1){
		return -1;
	}

	// check all tracks played
	int i=0;
	QList<int> unplayed_tracks;
	for(MetaData& md : m->tracks)
	{
		UniqueId unique_id = md.uniqueId();
		if(!m->shuffleHistory.contains(unique_id)) {
			unplayed_tracks << i;
		}

		i++;
	}

	// no random track to play
	if(unplayed_tracks.isEmpty())
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
		RandomGenerator rnd;
		int left_tracks_idx = rnd.getNumber(0, unplayed_tracks.size() - 1);

		return unplayed_tracks[left_tracks_idx];
	}
}

bool PlaylistImpl::wakeUp()
{
	int idx = trackIndexBeforeStop();

	if(Util::between(idx, count()))
	{
		return changeTrack(idx);
	}

	return false;
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

void Playlist::Playlist::setCurrentScannedFile(const QString& current_scanned_file)
{
	emit sigCurrentScannedFileChanged(current_scanned_file);
}

void Playlist::Playlist::reverse()
{
	std::reverse(m->tracks.begin(), m->tracks.end());
	setChanged(true);
}

void PlaylistImpl::enableAll()
{
	for(MetaData& md : m->tracks)
	{
		md.setDisabled(false);
	}

	setChanged(true);
}

int PlaylistImpl::createPlaylist(const MetaDataList& v_md)
{
	if(PlaylistMode::isActiveAndEnabled(m->playlistMode.append()) == false)
	{
		m->tracks.clear();
		m->shuffleHistory.clear();
	}

	m->tracks << v_md;

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
	if( m->playlistMode.shuffle() != mode.shuffle())
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
	MilliSeconds dur_ms  = std::accumulate(m->tracks.begin(), m->tracks.end(), 0, [](MilliSeconds time, const MetaData& md){
		return time + md.durationMs();
	});

	return dur_ms;
}

int PlaylistImpl::currentTrackIndex() const
{
	if(m->playingUniqueId == 0){
		return -1;
	}

	return Util::Algorithm::indexOf(m->tracks, [this](const MetaData& md)
	{
		return (md.uniqueId() == m->playingUniqueId);
	});
}

bool PlaylistImpl::currentTrack(MetaData& md) const
{
	int idx = currentTrackIndex();
	if(!Util::between(idx, m->tracks)) {
		return false;
	}

	md = m->tracks[idx];
	return true;
}

void Playlist::Playlist::setCurrentTrack(int idx)
{
	if(!Util::between(idx, m->tracks)) {
		m->playingUniqueId = 0;
	}

	else {
		m->playingUniqueId = m->tracks[idx].uniqueId();
	}

	emit sigCurrentTrackChanged(idx);
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

bool PlaylistImpl::isStoreable() const
{
	return (m->type == PlaylistType::Std);
}

void PlaylistImpl::settingPlaylistModeChanged()
{
	setMode( GetSetting(Set::PL_Mode) );
}

const MetaDataList& PlaylistImpl::tracks() const
{
	return m->tracks;
}

const MetaData& PlaylistImpl::track(int idx) const
{
	if(idx >= 0 && idx < m->tracks.count())
	{
		return m->tracks[idx];
	}

	return m->dummyTrack;
}
