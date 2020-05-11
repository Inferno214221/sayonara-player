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

#include "PlayManager.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Interfaces/Notification/NotificationHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/FileUtils.h"

#include <QDateTime>
#include <QTime>

#include <array>

const int InvalidTimeStamp=-1;

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

struct PlayManager::Private
{
	MetaData				md;
	RingBuffer<QString, 3>	ringBuffer;
	int						currentTrackIndex;
	MilliSeconds			positionMs;
	MilliSeconds			initialPositionMs;
	MilliSeconds			trackPlaytimeMs;
	PlayState				playstate;

	Private()
	{
		reset();
		playstate = PlayState::FirstStartup;
	}

	void reset()
	{
		md = MetaData();
		ringBuffer.clear();
		currentTrackIndex = -1;
		positionMs = 0;
		trackPlaytimeMs = 0;
		initialPositionMs = InvalidTimeStamp;
		playstate = PlayState::Stopped;
	}
};

PlayManager::PlayManager(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	bool loadPlaylist = (GetSetting(Set::PL_LoadSavedPlaylists) || GetSetting(Set::PL_LoadTemporaryPlaylists));
	bool loadLastTrack = GetSetting(Set::PL_LoadLastTrack);
	bool rememberLastTime = GetSetting(Set::PL_RememberTime);

	if(loadPlaylist && loadLastTrack)
	{
		if(rememberLastTime) {
			m->initialPositionMs = GetSetting(Set::Engine_CurTrackPos_s) * 1000;
		}

		else {
			m->initialPositionMs = 0;
		}
	}

	else {
		m->initialPositionMs = InvalidTimeStamp;
	}

	auto* mdcn = Tagging::ChangeNotifier::instance();
	connect(mdcn, &Tagging::ChangeNotifier::sigMetadataChanged, this, &PlayManager::trackMetadataChanged);
	connect(mdcn, &Tagging::ChangeNotifier::sigMetadataDeleted, this, &PlayManager::tracksDeleted);
}

PlayManager::~PlayManager() = default;

PlayState PlayManager::playstate() const
{
	return m->playstate;
}

MilliSeconds PlayManager::currentPositionMs() const
{
	return m->positionMs;
}

MilliSeconds PlayManager::currentTrackPlaytimeMs() const
{
	return m->trackPlaytimeMs;
}

MilliSeconds PlayManager::initialPositionMs() const
{
	return m->initialPositionMs;
}

MilliSeconds PlayManager::durationMs() const
{
	return m->md.durationMs();
}

Bitrate PlayManager::bitrate() const
{
	return m->md.bitrate();
}

const MetaData& PlayManager::currentTrack() const
{
	return m->md;
}

int PlayManager::volume() const
{
	return GetSetting(Set::Engine_Vol);
}

bool PlayManager::isMuted() const
{
	return GetSetting(Set::Engine_Mute);
}

void PlayManager::play()
{
	m->playstate = PlayState::Playing;
	emit sigPlaystateChanged(m->playstate);
}

void PlayManager::wakeUp()
{
	emit sigWakeup();
}

void PlayManager::playPause()
{
	if(m->playstate == PlayState::Playing) {
		pause();
	}

	else if(m->playstate == PlayState::Stopped) {
		wakeUp();
	}

	else {
		play();
	}
}

void PlayManager::pause()
{
	m->playstate = PlayState::Paused;
	emit sigPlaystateChanged(m->playstate);
}

void PlayManager::previous()
{
	emit sigPrevious();
}

void PlayManager::next()
{
	emit sigNext();
}

void PlayManager::stop()
{
	m->reset();

	emit sigPlaystateChanged(m->playstate);
}

void PlayManager::record(bool b)
{
	if(GetSetting(SetNoDB::MP3enc_found)) {
		emit sigRecording(b);
	} else {
		emit sigRecording(false);
	}
}

void PlayManager::seekRelative(double percent)
{
	emit sigSeekedRelative(percent);
}

void PlayManager::seekRelativeMs(MilliSeconds ms)
{
	emit sigSeekedRelativeMs(ms);
}

void PlayManager::seekAbsoluteMs(MilliSeconds ms)
{
	emit sigSeekedAbsoluteMs(ms);
}

void PlayManager::setCurrentPositionMs(MilliSeconds ms)
{
	MilliSeconds difference = (ms - m->positionMs);
	if(difference > 0 && difference < 1000) {
		m->trackPlaytimeMs += difference;
	}

	m->positionMs = ms;

	SetSetting(Set::Engine_CurTrackPos_s, int(m->positionMs / 1000));

	emit sigPositionChangedMs(ms);
}

void PlayManager::changeCurrentTrack(const MetaData& md, int trackIdx)
{
	bool isFirstStart = (m->playstate == PlayState::FirstStartup);

	m->md = md;
	m->positionMs = 0;
	m->trackPlaytimeMs = 0;
	m->currentTrackIndex = trackIdx;
	m->ringBuffer.clear();

	// initial position is outdated now and never needed again
	if(m->initialPositionMs >= 0)
	{
		int oldIndex = GetSetting(Set::PL_LastTrack);
		if(oldIndex != m->currentTrackIndex) {
			m->initialPositionMs = InvalidTimeStamp;
		}
	}

	// play or stop
	if(m->currentTrackIndex >= 0)
	{
		emit sigCurrentTrackChanged(m->md);
		emit sigTrackIndexChanged(m->currentTrackIndex);

		if(!isFirstStart)
		{
			play();

			if( (md.radioMode() != RadioMode::Off) &&
					GetSetting(Set::Engine_SR_Active) &&
					GetSetting(Set::Engine_SR_AutoRecord) )
			{
				record(true);
			}
		}
	}

	else {
		spLog(Log::Info, this) << "Playlist finished";
		emit sigPlaylistFinished();
		stop();
	}

	if(!isFirstStart)
	{
		// save last track
		if(md.databaseId() == 0) {
			SetSetting(Set::PL_LastTrack, m->currentTrackIndex);
		}

		else {
			SetSetting(Set::PL_LastTrack, -1);
		}
	}

	// show notification
	if(GetSetting(Set::Notification_Show))
	{
		if(m->currentTrackIndex > -1 && !m->md.filepath().isEmpty())
		{
			NotificationHandler::instance()->notify(m->md);
		}
	}
}

void PlayManager::changeCurrentMetadata(const MetaData& md)
{
	if(m->md.radioMode() == RadioMode::Podcast)
	{
		if(!m->md.title().isEmpty() && !m->md.artist().isEmpty() && !m->md.album().isEmpty())
		{
			return;
		}
	}

	MetaData mdOld = m->md;
	m->md = md;

	const QString str = md.title() + md.artist() + md.album();
	bool hasData = m->ringBuffer.contains(str);

	if(!hasData)
	{
		if(GetSetting(Set::Notification_Show)) {
			NotificationHandler::instance()->notify(m->md);
		}

		// only insert www tracks into the buffer
		if( m->ringBuffer.count() > 0 && Util::File::isWWW(md.filepath()))
		{
			mdOld.setAlbum("");
			mdOld.setDisabled(true);
			mdOld.setFilepath("");

			const QTime time = QTime::currentTime();
			mdOld.setDurationMs((time.hour() * 60 + time.minute()) * 1000);

			emit sigStreamFinished(mdOld);

			m->trackPlaytimeMs = 0;
		}
	}

	emit sigCurrentMetadataChanged();
}

void PlayManager::setTrackReady()
{
	if(m->initialPositionMs == InvalidTimeStamp) {
		return;
	}

	spLog(Log::Debug, this) << "Track ready, Start at " << m->initialPositionMs / 1000 << "ms";
	if(m->initialPositionMs != 0)
	{
		this->seekAbsoluteMs(m->initialPositionMs);
	}

	m->initialPositionMs = InvalidTimeStamp;

	if(GetSetting(Set::PL_StartPlaying)){
		play();
	}

	else {
		pause();
	}
}

void PlayManager::setTrackFinished()
{
	next();
}

void PlayManager::buffering(int progress)
{
	emit sigBuffering(progress);
}

void PlayManager::volumeUp()
{
	setVolume(GetSetting(Set::Engine_Vol) + 5);
}

void PlayManager::volumeDown()
{
	setVolume(GetSetting(Set::Engine_Vol) - 5);
}

void PlayManager::setVolume(int vol)
{
	vol = std::min(vol, 100);
	vol = std::max(vol, 0);
	SetSetting(Set::Engine_Vol, vol);
	emit sigVolumeChanged(vol);
}

void PlayManager::setMute(bool b)
{
	SetSetting(Set::Engine_Mute, b);
	emit sigMuteChanged(b);
}

void PlayManager::toggleMute()
{
	bool muted = GetSetting(Set::Engine_Mute);
	setMute(!muted);
}

void PlayManager::error(const QString& message)
{
	emit sigError(message);
}

void PlayManager::changeDuration(MilliSeconds ms)
{
	m->md.setDurationMs(ms);
	emit sigDurationChangedMs();
}

void PlayManager::changeBitrate(Bitrate br)
{
	m->md.setBitrate(br);
	emit sigBitrateChanged();
}

void PlayManager::shutdown()
{
	if(m->playstate == PlayState::Stopped)
	{
		SetSetting(Set::PL_LastTrack, -1);
		SetSetting(Set::Engine_CurTrackPos_s, 0);
	}

	else {
		SetSetting(Set::Engine_CurTrackPos_s, int(m->positionMs / 1000));
	}
}

void PlayManager::trackMetadataChanged()
{
	auto* mdcn = static_cast<Tagging::ChangeNotifier*>(sender());

	const QList<MetaDataPair> changedMetadata = mdcn->changedMetadata();
	for(const MetaDataPair& trackPair : changedMetadata)
	{
		bool isSamePath = m->md.isEqual(trackPair.first);
		if(isSamePath)
		{
			this->changeCurrentMetadata(trackPair.second);
			return;
		}
	}
}

void PlayManager::tracksDeleted()
{
	auto* mdcn = static_cast<Tagging::ChangeNotifier*>(sender());

	const MetaDataList deletedTracks = mdcn->deletedMetadata();
	if(deletedTracks.contains(m->md)) {
		stop();
	}
}
