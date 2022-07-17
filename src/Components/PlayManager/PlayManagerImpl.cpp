/* PlayManager.cpp */

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

#include "PlayManagerImpl.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Interfaces/Notification/NotificationHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QDateTime>
#include <QTimer>

#include <array>
#include <optional>

namespace
{
	constexpr const auto InvalidTimeStamp {-1};
	constexpr const auto VolumeDelta {5};
	constexpr const auto RingbufferSize {3};

	template<int MaxItemCount>
	class RingBuffer
	{
		private:
			size_t mCurrentIndex {0};
			size_t mItemCount {0};
			std::array<QString, MaxItemCount> mData;

		public:
			void clear()
			{
				mCurrentIndex = 0;
				mItemCount = 0;
			}

			void insert(const QString& item)
			{
				mData[mCurrentIndex] = item; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
				mCurrentIndex = (mCurrentIndex + 1) % MaxItemCount;
				mItemCount = std::min<size_t>(MaxItemCount, mItemCount + 1);
			}

			[[nodiscard]] bool contains(const QString& item) const
			{
				return Util::Algorithm::contains(mData, [&](const auto& element) {
					return (element == item);
				});
			}
	};

	bool isTrackValidPodcast(const MetaData& track)
	{
		return (track.radioMode() == RadioMode::Podcast) &&
		       (!track.title().isEmpty()) &&
		       (!track.artist().isEmpty()) &&
		       (!track.album().isEmpty());
	}

	QString getTrackHash(const MetaData& track)
	{
		return (track.title() + track.artist() + track.album() + track.filepath());
	}

	MilliSeconds currentTimeToDuration()
	{
		const auto time = QTime::currentTime();
		return (time.hour() * 60 + time.minute()) * 1000; // NOLINT(readability-magic-numbers)
	}

	MetaData prepareTrackForStreamHistory(MetaData track)
	{
		track.setAlbum("");
		track.setDisabled(true);
		track.setDurationMs(currentTimeToDuration());

		return track;
	}

	void showNotification(const MetaData& track)
	{
		if(GetSetting(Set::Notification_Show))
		{
			NotificationHandler::instance()->notify(track);
		}
	}

	bool isStreamRecordableTrack(const MetaData& track)
	{
		return (track.radioMode() != RadioMode::Off) &&
		       GetSetting(Set::Engine_SR_Active) &&
		       GetSetting(Set::Engine_SR_AutoRecord);
	}

	void startPaused(PlayManager* playManager)
	{
		if(GetSetting(Set::PL_StartPlayingWorkaround_Issue263))
		{
			playManager->play();
			QTimer::singleShot(100, [playManager]() { // NOLINT(readability-magic-numbers)
				playManager->pause();
			});
		}

		else
		{
			playManager->pause();
		}
	}
} // namespace

struct PlayManagerImpl::Private
{
	MetaData currentTrack;
	RingBuffer<RingbufferSize> ringBuffer;
	int currentTrackIndex {-1};
	MilliSeconds positionMs {0};
	std::optional<MilliSeconds> initialPositionMs;
	MilliSeconds trackPlaytimeMs {0};
	PlayState playstate {PlayState::FirstStartup};

	Private()
	{
		const auto loadPlaylist = (GetSetting(Set::PL_LoadSavedPlaylists) ||
		                           GetSetting(Set::PL_LoadTemporaryPlaylists));
		const auto loadLastTrack = GetSetting(Set::PL_LoadLastTrack);
		const auto rememberLastTime = GetSetting(Set::PL_RememberTime);

		if(loadPlaylist && loadLastTrack)
		{
			initialPositionMs = rememberLastTime ?
			                    (GetSetting(Set::Engine_CurTrackPos_s) * 1000) : // NOLINT(readability-magic-numbers)
			                    0;
		}
	}

	void reset()
	{
		currentTrack = MetaData();
		ringBuffer.clear();
		currentTrackIndex = -1;
		positionMs = 0;
		trackPlaytimeMs = 0;
		initialPositionMs.reset();
		playstate = PlayState::Stopped;
	}
};

PlayManagerImpl::PlayManagerImpl(QObject* parent) :
	PlayManager(parent)
{
	m = Pimpl::make<Private>();

	auto* mdcn = Tagging::ChangeNotifier::instance();
	connect(mdcn, &Tagging::ChangeNotifier::sigMetadataChanged, this, &PlayManagerImpl::trackMetadataChanged);
	connect(mdcn, &Tagging::ChangeNotifier::sigMetadataDeleted, this, &PlayManagerImpl::tracksDeleted);
}

PlayManagerImpl::~PlayManagerImpl() = default;

PlayState PlayManagerImpl::playstate() const
{
	return m->playstate;
}

MilliSeconds PlayManagerImpl::currentPositionMs() const
{
	return m->positionMs;
}

MilliSeconds PlayManagerImpl::currentTrackPlaytimeMs() const
{
	return m->trackPlaytimeMs;
}

MilliSeconds PlayManagerImpl::initialPositionMs() const
{
	return m->initialPositionMs.has_value()
	       ? m->initialPositionMs.value()
	       : InvalidTimeStamp;
}

MilliSeconds PlayManagerImpl::durationMs() const
{
	return m->currentTrack.durationMs();
}

Bitrate PlayManagerImpl::bitrate() const
{
	return m->currentTrack.bitrate();
}

const MetaData& PlayManagerImpl::currentTrack() const
{
	return m->currentTrack;
}

int PlayManagerImpl::volume() const
{
	return GetSetting(Set::Engine_Vol);
}

bool PlayManagerImpl::isMuted() const
{
	return GetSetting(Set::Engine_Mute);
}

void PlayManagerImpl::play()
{
	m->playstate = PlayState::Playing;
	emit sigPlaystateChanged(m->playstate);
}

void PlayManagerImpl::wakeUp()
{
	emit sigWakeup();
}

void PlayManagerImpl::playPause()
{
	if(m->playstate == PlayState::Playing)
	{
		pause();
	}

	else if(m->playstate == PlayState::Stopped)
	{
		wakeUp();
	}

	else
	{
		play();
	}
}

void PlayManagerImpl::pause()
{
	m->playstate = PlayState::Paused;
	emit sigPlaystateChanged(m->playstate);
}

void PlayManagerImpl::previous()
{
	emit sigPrevious();
}

void PlayManagerImpl::next()
{
	emit sigNext();
}

void PlayManagerImpl::stop()
{
	m->reset();

	emit sigPlaystateChanged(m->playstate);
}

void PlayManagerImpl::record(bool b)
{
	emit sigRecording(b && GetSetting(SetNoDB::MP3enc_found));
}

void PlayManagerImpl::seekRelative(double percent)
{
	emit sigSeekedRelative(percent);
}

void PlayManagerImpl::seekRelativeMs(MilliSeconds ms)
{
	emit sigSeekedRelativeMs(ms);
}

void PlayManagerImpl::seekAbsoluteMs(MilliSeconds ms)
{
	emit sigSeekedAbsoluteMs(ms);
}

void PlayManagerImpl::setCurrentPositionMs(MilliSeconds ms)
{
	const auto differenceMs = (ms - m->positionMs);
	if((differenceMs > 0) && (differenceMs < 1000)) // NOLINT(readability-magic-numbers)
	{
		m->trackPlaytimeMs += differenceMs;
	}

	m->positionMs = ms;

	SetSetting(Set::Engine_CurTrackPos_s, static_cast<int>(m->positionMs / 1000));

	emit sigPositionChangedMs(ms);
}

void PlayManagerImpl::changeCurrentTrack(const MetaData& track, int trackIdx)
{
	const auto isFirstStartup = (m->playstate == PlayState::FirstStartup);
	const auto oldTrackHash = getTrackHash(m->currentTrack);

	m->currentTrack = track;
	m->positionMs = 0;
	m->trackPlaytimeMs = 0;
	m->currentTrackIndex = trackIdx;
	m->ringBuffer.clear();

	if(m->currentTrack.radioMode() == RadioMode::Station)
	{
		const auto trackHash = getTrackHash(m->currentTrack);
		m->ringBuffer.insert(trackHash);
	}

	if(!oldTrackHash.isEmpty() && (oldTrackHash != getTrackHash(m->currentTrack)))
	{
		m->initialPositionMs.reset();
	}

	// play or stop
	if(m->currentTrackIndex >= 0)
	{
		emit sigCurrentTrackChanged(m->currentTrack);
		emit sigTrackIndexChanged(m->currentTrackIndex);

		if(!isFirstStartup)
		{
			play();

			if(isStreamRecordableTrack(track))
			{
				record(true);
			}
		}
	}

	else
	{
		spLog(Log::Info, this) << "Playlist finished";

		emit sigPlaylistFinished();
		stop();
	}

	if(!isFirstStartup)
	{
		// save last track
		const auto currentIndex = (track.databaseId() == 0) ? m->currentTrackIndex : -1;
		SetSetting(Set::PL_LastTrack, currentIndex);
	}

	// show notification
	if((m->currentTrackIndex > -1) && (!m->currentTrack.filepath().isEmpty()))
	{
		showNotification(m->currentTrack);
	}
}

void PlayManagerImpl::changeCurrentMetadata(const MetaData& newMetadata)
{
	if(isTrackValidPodcast(m->currentTrack))
	{
		return;
	}

	auto oldMetadata = std::move(m->currentTrack);
	m->currentTrack = newMetadata;

	if(m->currentTrack.radioMode() == RadioMode::Station)
	{
		const auto trackHash = getTrackHash(m->currentTrack);
		const auto ignoreNewTrack = m->ringBuffer.contains(trackHash);
		if(!ignoreNewTrack)
		{
			m->trackPlaytimeMs = 0;

			oldMetadata = prepareTrackForStreamHistory(std::move(oldMetadata));

			emit sigStreamFinished(oldMetadata);
			showNotification(m->currentTrack);
		}

		m->ringBuffer.insert(trackHash);
	}

	emit sigCurrentMetadataChanged();
}

void PlayManagerImpl::setTrackReady()
{
	if(m->initialPositionMs)
	{
		const auto initialPositionMs = m->initialPositionMs.value();
		if(initialPositionMs > 0)
		{
			// NOLINTNEXTLINE(readability-magic-numbers)
			spLog(Log::Debug, this) << "Track ready, Start at " << (initialPositionMs / 1000) << " sec";
			seekAbsoluteMs(initialPositionMs);
		}

		m->initialPositionMs.reset();

		const auto startPlaying = GetSetting(Set::PL_StartPlaying);
		if(!startPlaying)
		{
			startPaused(this);
		}

		else
		{
			play();
		}
	}
}

void PlayManagerImpl::setTrackFinished()
{
	next();
}

void PlayManagerImpl::buffering(int progress)
{
	emit sigBuffering(progress);
}

void PlayManagerImpl::volumeUp()
{
	setVolume(GetSetting(Set::Engine_Vol) + VolumeDelta);
}

void PlayManagerImpl::volumeDown()
{
	setVolume(GetSetting(Set::Engine_Vol) - VolumeDelta);
}

void PlayManagerImpl::setVolume(int vol)
{
	vol = std::min(vol, 100); // NOLINT(readability-magic-numbers)
	vol = std::max(vol, 0);
	SetSetting(Set::Engine_Vol, vol);
	emit sigVolumeChanged(vol);
}

void PlayManagerImpl::setMute(bool b)
{
	SetSetting(Set::Engine_Mute, b);
	emit sigMuteChanged(b);
}

void PlayManagerImpl::toggleMute()
{
	setMute(!GetSetting(Set::Engine_Mute));
}

void PlayManagerImpl::error(const QString& message)
{
	emit sigError(message);
}

void PlayManagerImpl::changeDuration(MilliSeconds ms)
{
	m->currentTrack.setDurationMs(ms);
	emit sigDurationChangedMs();
}

void PlayManagerImpl::changeBitrate(Bitrate br)
{
	m->currentTrack.setBitrate(br);
	emit sigBitrateChanged();
}

void PlayManagerImpl::shutdown()
{
	if(m->playstate == PlayState::Stopped)
	{
		SetSetting(Set::PL_LastTrack, -1);
		SetSetting(Set::Engine_CurTrackPos_s, 0);
	}

	else
	{
		SetSetting(Set::Engine_CurTrackPos_s, static_cast<int>(m->positionMs / 1000));
	}
}

void PlayManagerImpl::trackMetadataChanged()
{
	auto* changeNotifier = dynamic_cast<Tagging::ChangeNotifier*>(sender());

	const auto& changedMetadata = changeNotifier->changedMetadata();
	for(const auto& [oldTrack, newTrack]: changedMetadata)
	{
		const auto isSamePath = m->currentTrack.isEqual(oldTrack);
		if(isSamePath)
		{
			this->changeCurrentMetadata(newTrack);
			return;
		}
	}
}

void PlayManagerImpl::tracksDeleted()
{
	auto* changeNotifier = dynamic_cast<Tagging::ChangeNotifier*>(sender());

	const auto& deletedTracks = changeNotifier->deletedMetadata();
	if(deletedTracks.contains(m->currentTrack))
	{
		stop();
	}
}