/* Playlist.cpp */

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

#include "Playlist.h"
#include "PlaylistModifiers.h"
#include "PlaylistShuffleHistory.h"
#include "PlaylistStopBehavior.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Tagging/ChangeNotifier.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

#include <QMultiHash>

namespace Playlist
{
	namespace
	{
		QMultiHash<QString, int> createFilepathIndexHash(const MetaDataList& tracks)
		{
			auto result = QMultiHash<QString, int>();

			auto i = 0;
			for(auto it = tracks.begin(); it != tracks.end(); it++, i++)
			{
				result.insert(it->filepath(), i);
			}

			return result;
		}

		int getNextTrackIndex(const int currentTrackIndex, const int trackCount, const PlaylistMode& playlistMode,
		                      const ShuffleHistory& shuffleHistory)
		{
			const auto rep1 = PlaylistMode::isActiveAndEnabled(playlistMode.rep1());
			const auto shuffle = PlaylistMode::isActiveAndEnabled(playlistMode.shuffle());
			const auto repAll = (PlaylistMode::isActiveAndEnabled(playlistMode.repAll()));
			const auto isLastTrack = (currentTrackIndex == trackCount - 1);

			if((currentTrackIndex == -1) && !shuffle)
			{
				return 0;
			}

			if(rep1)
			{
				return currentTrackIndex;
			}

			if(shuffle)
			{
				return shuffleHistory.nextTrackIndex(repAll);
			}

			if(isLastTrack)
			{
				return repAll ? 0 : -1;
			}

			return currentTrackIndex + 1;
		}

		QList<int> findTracks(const MetaDataList& tracks, const QString& path)
		{
			if(path.isEmpty())
			{
				return {};
			}

			QList<int> ret;
			for(auto it = tracks.begin(); it != tracks.end(); it++)
			{
				if(it->filepath() == path)
				{
					ret << static_cast<int>(std::distance(tracks.begin(), it));
				}
			}

			return ret;
		}
	}

	struct Playlist::Private
	{
		Tagging::ChangeNotifier* metadataChangeNotifier {Tagging::ChangeNotifier::instance()};

		PlayManager* playManager;
		MetaDataList tracks;
		ShuffleHistory shuffleHistory;
		StopBehavior stopBehavior;
		UniqueId playingUniqueId {0U};
		PlaylistMode playlistMode;
		int playlistIndex;
		bool playlistChanged {false};
		bool busy {false};

		Private(int playlistIndex, const PlaylistMode& playlistMode, PlayManager* playManager, Playlist* playlist) :
			playManager(playManager),
			shuffleHistory(playlist),
			stopBehavior(playlist),
			playlistMode(playlistMode),
			playlistIndex(playlistIndex) {}
	};

	Playlist::Playlist(int playlistIndex, const QString& name, PlayManager* playManager) :
		Playlist::DBInterface(name)
	{
		m = Pimpl::make<::Playlist::Playlist::Private>(playlistIndex, GetSetting(Set::PL_Mode), playManager, this);

		connect(m->metadataChangeNotifier,
		        &Tagging::ChangeNotifier::sigMetadataChanged,
		        this,
		        &Playlist::metadataChanged);
		connect(m->metadataChangeNotifier,
		        &Tagging::ChangeNotifier::sigMetadataDeleted,
		        this,
		        &Playlist::metadataDeleted);

		connect(m->playManager, &PlayManager::sigCurrentMetadataChanged, this, &Playlist::currentMetadataChanged);
		connect(m->playManager, &PlayManager::sigDurationChangedMs, this, &Playlist::durationChanged);

		ListenSetting(Set::PL_Mode, Playlist::settingPlaylistModeChanged);
	}

	Playlist::~Playlist() = default;

	bool Playlist::changeTrack(int index, const MilliSeconds positionMs)
	{
		const auto oldIndex = currentTrackIndex();
		m->stopBehavior.setTrackIndexBeforeStop(-1);
		setCurrentTrack(index);

		if(!Util::between(index, m->tracks))
		{
			stop();
			m->stopBehavior.setTrackIndexBeforeStop(-1);
			return false;
		}

		m->shuffleHistory.addTrack(m->tracks[index]);

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

	void Playlist::metadataDeleted()
	{
		const auto deletedTracks = m->metadataChangeNotifier->deletedMetadata();
		const auto it = std::remove_if(m->tracks.begin(), m->tracks.end(), [&](const auto& track) {
			return Util::Algorithm::contains(deletedTracks, [&](const auto& tmpTrack) {
				return (track.isEqual(tmpTrack));
			});
		});

		m->tracks.erase(it, m->tracks.end());
		emit sigItemsChanged(index());
	}

	void Playlist::metadataChanged()
	{
		const auto filepathIndexMap = createFilepathIndexHash(tracks());

		const auto& changedTracks = m->metadataChangeNotifier->changedMetadata();
		for(const auto& changedPair: changedTracks)
		{
			const auto filepath = changedPair.first.filepath();
			const auto indexes = filepathIndexMap.values(filepath);
			for(const auto& index: indexes)
			{
				replaceTrack(index, changedPair.second);
			}
		}

		emit sigItemsChanged(this->index());
	}

	void Playlist::currentMetadataChanged()
	{
		const auto currentTrack = m->playManager->currentTrack();
		const auto indexList = findTracks(m->tracks, currentTrack.filepath());
		for(const auto index: indexList)
		{
			replaceTrack(index, currentTrack);
		}
	}

	void Playlist::durationChanged()
	{
		const auto currentTrack = m->playManager->currentTrack();
		const auto indexList = findTracks(m->tracks, currentTrack.filepath());

		for(const auto index: indexList)
		{
			auto track = (m->tracks[index]);
			track.setDurationMs(std::max<MilliSeconds>(0, currentTrack.durationMs()));
			replaceTrack(index, track);
		}
	}

	void Playlist::replaceTrack(const int index, const MetaData& track)
	{
		if(!Util::between(index, m->tracks) || (m->tracks[index].isDisabled()))
		{
			return;
		}

		const auto oldUniqueId = m->tracks[index].uniqueId();
		const auto isCurrent = (oldUniqueId == m->playingUniqueId);
		const auto isEnabled = Util::File::checkFile(track.filepath()) && !track.isDisabled();

		m->shuffleHistory.replaceTrack(m->tracks[index], track);
		m->tracks[index] = track;
		m->tracks[index].setDisabled(!isEnabled);

		if(isCurrent)
		{
			m->playingUniqueId = m->tracks[index].uniqueId();
		}

		emit sigItemsChanged(this->index());
	}

	void Playlist::play()
	{
		if(currentTrackIndex() < 0)
		{
			next();
		}
	}

	void Playlist::stop()
	{
		m->shuffleHistory.clear();

		const auto currentTrack = currentTrackIndex();
		if(currentTrack >= 0)
		{
			m->stopBehavior.setTrackIndexBeforeStop(currentTrack);
			m->playingUniqueId = 0;
		}

		emit sigTrackChanged(currentTrack, -1);
	}

	void Playlist::fwd()
	{
		auto currentMode = m->playlistMode;
		const auto oldMode = m->playlistMode;

		// temp. disable rep1 as we want a new track
		currentMode.setRep1(false);
		setMode(currentMode);

		next();

		setMode(oldMode);
	}

	void Playlist::bwd()
	{
		constexpr const auto minimalOffset = 2000;
		if(m->playManager->currentPositionMs() > minimalOffset)
		{
			m->playManager->seekAbsoluteMs(0);
			return;
		}

		if(PlaylistMode::isActiveAndEnabled(m->playlistMode.shuffle()))
		{
			const auto index = m->shuffleHistory.previousShuffleIndex();
			if(index >= 0)
			{
				m->shuffleHistory.popBack();
				changeTrack(index);
				m->shuffleHistory.popBack();
				return;
			}
		}

		m->shuffleHistory.clear();
		changeTrack(currentTrackIndex() - 1);
	}

	void Playlist::next()
	{
		if(m->tracks.isEmpty())
		{
			stop();
			m->stopBehavior.setTrackIndexBeforeStop(-1);
			return;
		}

		const auto nextIndex =
			getNextTrackIndex(currentTrackIndex(), m->tracks.count(), m->playlistMode, m->shuffleHistory);
		changeTrack(nextIndex);
	}

	bool Playlist::wakeUp()
	{
		const auto index = m->stopBehavior.trackIndexBeforeStop();
		return (Util::between(index, count(*this)))
		       ? changeTrack(index)
		       : false; // NOLINT
	}

	int Playlist::createPlaylist(const MetaDataList& tracks)
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

	int Playlist::index() const { return m->playlistIndex; }

	void Playlist::setIndex(int idx) { m->playlistIndex = idx; }

	PlaylistMode Playlist::mode() const { return m->playlistMode; }

	void Playlist::setMode(const PlaylistMode& mode)
	{
		if(m->playlistMode.shuffle() != mode.shuffle())
		{
			m->shuffleHistory.clear();
		}

		m->playlistMode = mode;
	}

	bool Playlist::isBusy() const { return m->busy; }

	void Playlist::setBusy(bool busy)
	{
		m->busy = busy;
		emit sigBusyChanged(busy);
	}

	int Playlist::currentTrackIndex() const
	{
		if(m->playingUniqueId == 0)
		{
			return -1;
		}

		return Util::Algorithm::indexOf(m->tracks, [&](const auto& track) {
			return (track.uniqueId() == m->playingUniqueId);
		});
	}

	void Playlist::setCurrentTrack(const int index)
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

	bool Playlist::wasChanged() const { return m->playlistChanged; }

	void Playlist::setChanged(const bool b)
	{
		m->stopBehavior.restoreTrackBeforeStop();
		m->playlistChanged = b;

		emit sigItemsChanged(m->playlistIndex);
	}

	void Playlist::resetChangedStatus()
	{
		setChanged(false);
	}

	void Playlist::settingPlaylistModeChanged()
	{
		setMode(GetSetting(Set::PL_Mode));
	}

	const MetaDataList& Playlist::tracks() const { return m->tracks; }

	void Playlist::modifyTracks(Modificator&& modificator)
	{
		if(!isLocked())
		{
			m->tracks = modificator(std::move(m->tracks));
			setChanged(true);
		}
	}
}

