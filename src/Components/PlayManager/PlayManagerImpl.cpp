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
#include "Utils/FileUtils.h"

#include <QDateTime>

#include <array>

constexpr const auto InvalidTimeStamp {-1};

template<typename T, int MAX_ITEM_COUNT>
class RingBuffer
{
	private:
		size_t mCurrentIndex;
		size_t mItemCount;
		std::array<T, MAX_ITEM_COUNT> mData;

	public:
		RingBuffer()
		{
			clear();
		}

		void clear()
		{
			mCurrentIndex = 0;
			mItemCount = 0;
		}

		void insert(const T& item)
		{
			mData[mCurrentIndex] = item;
			mCurrentIndex = (mCurrentIndex + 1) % MAX_ITEM_COUNT;
			mItemCount = std::min<size_t>(MAX_ITEM_COUNT, mItemCount + 1);
		}

		bool contains(const T& item) const
		{
			auto it = std::find(mData.begin(), mData.end(), item);
			return (it != mData.end());
		}

		int count() const
		{
			return int(mItemCount);
		}

		bool isEmpty() const
		{
			return (count() == 0);
		}
};

struct PlayManagerImpl::Private
{
	MetaData currentTrack;
	RingBuffer<QString, 3> ringBuffer;
	int currentTrackIndex;
	MilliSeconds positionMs;
	MilliSeconds initialPositionMs;
	MilliSeconds trackPlaytimeMs;
	PlayState playstate;

	Private() :
		currentTrackIndex(-1),
		positionMs(0),
		initialPositionMs(InvalidTimeStamp),
		trackPlaytimeMs(0),
		playstate(PlayState::FirstStartup)
	{
		const auto loadPlaylist = (GetSetting(Set::PL_LoadSavedPlaylists) ||
		                           GetSetting(Set::PL_LoadTemporaryPlaylists));
		const auto loadLastTrack = GetSetting(Set::PL_LoadLastTrack);
		const auto rememberLastTime = GetSetting(Set::PL_RememberTime);

		if(loadPlaylist && loadLastTrack)
		{
			initialPositionMs = rememberLastTime ?
			                    (GetSetting(Set::Engine_CurTrackPos_s) * 1000) :
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
		initialPositionMs = InvalidTimeStamp;
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
	return m->initialPositionMs;
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
	if(differenceMs > 0 && differenceMs < 1000)
	{
		m->trackPlaytimeMs += differenceMs;
	}

	m->positionMs = ms;

	SetSetting(Set::Engine_CurTrackPos_s, int(m->positionMs / 1000));

	emit sigPositionChangedMs(ms);
}

void PlayManagerImpl::changeCurrentTrack(const MetaData& track, int trackIdx)
{
	const auto isFirstStart = (m->playstate == PlayState::FirstStartup);

	m->currentTrack = track;
	m->positionMs = 0;
	m->trackPlaytimeMs = 0;
	m->currentTrackIndex = trackIdx;
	m->ringBuffer.clear();

	// initial position is outdated now and never needed again
	if(m->initialPositionMs >= 0)
	{
		const auto oldIndex = GetSetting(Set::PL_LastTrack);
		if(oldIndex != m->currentTrackIndex)
		{
			m->initialPositionMs = InvalidTimeStamp;
		}
	}

	// play or stop
	if(m->currentTrackIndex >= 0)
	{
		emit sigCurrentTrackChanged(m->currentTrack);
		emit sigTrackIndexChanged(m->currentTrackIndex);

		if(!isFirstStart)
		{
			play();

			if((track.radioMode() != RadioMode::Off) &&
			   GetSetting(Set::Engine_SR_Active) &&
			   GetSetting(Set::Engine_SR_AutoRecord))
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

	if(!isFirstStart)
	{
		// save last track
		const auto currentIndex = (track.databaseId() == 0) ? m->currentTrackIndex : -1;
		SetSetting(Set::PL_LastTrack, currentIndex);
	}

	// show notification
	if(GetSetting(Set::Notification_Show))
	{
		if(m->currentTrackIndex > -1 && !m->currentTrack.filepath().isEmpty())
		{
			NotificationHandler::instance()->notify(m->currentTrack);
		}
	}
}

void PlayManagerImpl::changeCurrentMetadata(const MetaData& md)
{
	if(m->currentTrack.radioMode() == RadioMode::Podcast)
	{
		if(!m->currentTrack.title().isEmpty() &&
		   !m->currentTrack.artist().isEmpty() &&
		   !m->currentTrack.album().isEmpty())
		{
			return;
		}
	}

	auto lastTrack = std::move(m->currentTrack);
	m->currentTrack = md;

	const auto trackInfoString = md.title() + md.artist() + md.album();
	const auto hasData = m->ringBuffer.contains(trackInfoString);

	if(!hasData)
	{
		if(GetSetting(Set::Notification_Show))
		{
			NotificationHandler::instance()->notify(m->currentTrack);
		}

		// only insert www tracks into the buffer
		if(m->ringBuffer.count() > 0 && Util::File::isWWW(md.filepath()))
		{
			lastTrack.setAlbum("");
			lastTrack.setDisabled(true);
			lastTrack.setFilepath("");

			const auto time = QTime::currentTime();
			lastTrack.setDurationMs((time.hour() * 60 + time.minute()) * 1000);

			emit sigStreamFinished(lastTrack);

			m->trackPlaytimeMs = 0;
		}
	}

	emit sigCurrentMetadataChanged();
}

void PlayManagerImpl::setTrackReady()
{
	if(m->initialPositionMs == InvalidTimeStamp)
	{
		return;
	}

	spLog(Log::Debug, this) << "Track ready, Start at " << m->initialPositionMs / 1000 << "ms";
	if(m->initialPositionMs != 0)
	{
		this->seekAbsoluteMs(m->initialPositionMs);
	}

	m->initialPositionMs = InvalidTimeStamp;

	GetSetting(Set::PL_StartPlaying)
	? play()
	: pause();
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
	setVolume(GetSetting(Set::Engine_Vol) + 5);
}

void PlayManagerImpl::volumeDown()
{
	setVolume(GetSetting(Set::Engine_Vol) - 5);
}

void PlayManagerImpl::setVolume(int vol)
{
	vol = std::min(vol, 100);
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
		SetSetting(Set::Engine_CurTrackPos_s, int(m->positionMs / 1000));
	}
}

void PlayManagerImpl::trackMetadataChanged()
{
	auto* mdcn = static_cast<Tagging::ChangeNotifier*>(sender());

	const auto& changedMetadata = mdcn->changedMetadata();
	for(const auto& trackPair : changedMetadata)
	{
		const auto isSamePath = m->currentTrack.isEqual(trackPair.first);
		if(isSamePath)
		{
			this->changeCurrentMetadata(trackPair.second);
			return;
		}
	}
}

void PlayManagerImpl::tracksDeleted()
{
	auto* mdcn = static_cast<Tagging::ChangeNotifier*>(sender());

	const auto& deletedTracks = mdcn->deletedMetadata();
	if(deletedTracks.contains(m->currentTrack))
	{
		stop();
	}
}