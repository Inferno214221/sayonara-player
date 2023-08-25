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

#include "Components/Notification/NotificationHandler.h"
#include "Components/Tagging/ChangeNotifier.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"

#include <QDateTime>
#include <QTimer>

#include <array>
#include <optional>

namespace
{
	constexpr const auto InvalidTimeStamp {-1};
	constexpr const auto VolumeDelta {5};
	constexpr const auto RingbufferSize {12};

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

	void showNotification(NotificationHandler* notificationHandler, const MetaData& track)
	{
		if(GetSetting(Set::Notification_Show))
		{
			notificationHandler->notify(track);
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

class PlayManagerImpl :
	public PlayManager
{
	public:
		explicit PlayManagerImpl(NotificationHandler* notificationHandler, QObject* parent) :
			PlayManager(parent),
			m_notificationHandler {notificationHandler}
		{
			const auto loadPlaylist = (GetSetting(Set::PL_LoadSavedPlaylists) ||
			                           GetSetting(Set::PL_LoadTemporaryPlaylists));
			const auto loadLastTrack = GetSetting(Set::PL_LoadLastTrack);
			const auto rememberLastTime = GetSetting(Set::PL_RememberTime);

			if(loadPlaylist && loadLastTrack)
			{
				// NOLINTNEXTLINE(readability-magic-numbers)
				m_initialPositionMs = rememberLastTime ? (GetSetting(Set::Engine_CurTrackPos_s) * 1000)
				                                       : 0;
			}

			auto* mdcn = Tagging::ChangeNotifier::instance();
			connect(mdcn, &Tagging::ChangeNotifier::sigMetadataChanged, this, &PlayManagerImpl::trackMetadataChanged);
			connect(mdcn, &Tagging::ChangeNotifier::sigMetadataDeleted, this, &PlayManagerImpl::tracksDeleted);
		}

		~PlayManagerImpl() override = default;

		[[nodiscard]] PlayState playstate() const override
		{
			return m_playstate;
		}

		[[nodiscard]] MilliSeconds currentPositionMs() const override
		{
			return m_positionMs;
		}

		[[nodiscard]] MilliSeconds currentTrackPlaytimeMs() const override
		{
			return m_trackPlaytimeMs;
		}

		[[nodiscard]] MilliSeconds initialPositionMs() const override
		{
			return m_initialPositionMs.has_value()
			       ? m_initialPositionMs.value()
			       : InvalidTimeStamp;
		}

		[[nodiscard]] MilliSeconds durationMs() const override
		{
			return m_currentTrack.durationMs();
		}

		[[nodiscard]] Bitrate bitrate() const override
		{
			return m_currentTrack.bitrate();
		}

		[[nodiscard]] const MetaData& currentTrack() const override
		{
			return m_currentTrack;
		}

		[[nodiscard]] int volume() const override
		{
			return GetSetting(Set::Engine_Vol);
		}

		[[nodiscard]] bool isMuted() const override
		{
			return GetSetting(Set::Engine_Mute);
		}

		void play() override
		{
			m_playstate = PlayState::Playing;
			emit sigPlaystateChanged(m_playstate);
		}

		void wakeUp() override
		{
			emit sigWakeup();
		}

		void playPause() override
		{
			if(m_playstate == PlayState::Playing)
			{
				pause();
			}

			else if(m_playstate == PlayState::Stopped)
			{
				wakeUp();
			}

			else
			{
				play();
			}
		}

		void pause() override
		{
			m_playstate = PlayState::Paused;
			emit sigPlaystateChanged(m_playstate);
		}

		void previous() override
		{
			emit sigPrevious();
		}

		void next() override
		{
			emit sigNext();
		}

		void stop() override
		{
			reset();

			emit sigPlaystateChanged(m_playstate);
		}

		void record(const bool b) override
		{
			emit sigRecording(b && GetSetting(SetNoDB::MP3enc_found));
		}

		void seekRelative(const double percent) override
		{
			emit sigSeekedRelative(percent);
		}

		void seekRelativeMs(const MilliSeconds ms) override
		{
			emit sigSeekedRelativeMs(ms);
		}

		void seekAbsoluteMs(const MilliSeconds ms) override
		{
			emit sigSeekedAbsoluteMs(ms);
		}

		void setCurrentPositionMs(const MilliSeconds ms) override
		{
			const auto differenceMs = (ms - m_positionMs);
			if((differenceMs > 0) && (differenceMs < 1000)) // NOLINT(readability-magic-numbers)
			{
				m_trackPlaytimeMs += differenceMs;
			}

			m_positionMs = ms;

			SetSetting(Set::Engine_CurTrackPos_s, static_cast<int>(m_positionMs / 1000));

			emit sigPositionChangedMs(ms);
		}

		void changeCurrentTrack(const MetaData& track, const int trackIdx) override
		{
			const auto isFirstStartup = (m_playstate == PlayState::FirstStartup);
			const auto oldTrackHash = getTrackHash(m_currentTrack);

			m_currentTrack = track;
			m_positionMs = 0;
			m_trackPlaytimeMs = 0;
			m_currentTrackIndex = trackIdx;
			m_ringBuffer.clear();

			if(m_currentTrack.radioMode() == RadioMode::Station)
			{
				const auto trackHash = getTrackHash(m_currentTrack);
				m_ringBuffer.insert(trackHash);
			}

			if(!oldTrackHash.isEmpty() && (oldTrackHash != getTrackHash(m_currentTrack)))
			{
				m_initialPositionMs.reset();
			}

			// play or stop
			if(m_currentTrackIndex >= 0)
			{
				emit sigCurrentTrackChanged(m_currentTrack);
				emit sigTrackIndexChanged(m_currentTrackIndex);

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
				const auto currentIndex = (track.databaseId() == 0) ? m_currentTrackIndex : -1;
				SetSetting(Set::PL_LastTrack, currentIndex);
			}

			// show notification
			if((m_currentTrackIndex > -1) && (!m_currentTrack.filepath().isEmpty()))
			{
				showNotification(m_notificationHandler, m_currentTrack);
			}
		}

		void changeCurrentMetadata(const MetaData& newMetadata) override
		{
			if(isTrackValidPodcast(m_currentTrack))
			{
				return;
			}

			auto oldMetadata = std::move(m_currentTrack);
			m_currentTrack = newMetadata;

			if(m_currentTrack.radioMode() == RadioMode::Station)
			{
				const auto trackHash = getTrackHash(m_currentTrack);
				const auto ignoreNewTrack = m_ringBuffer.contains(trackHash);
				if(!ignoreNewTrack)
				{
					m_trackPlaytimeMs = 0;

					oldMetadata = prepareTrackForStreamHistory(std::move(oldMetadata));

					emit sigStreamFinished(oldMetadata);
					showNotification(m_notificationHandler, m_currentTrack);

					spLog(Log::Info, this) << "Cannot ignore " << trackHash;
					m_ringBuffer.insert(trackHash);
				}
			}

			emit sigCurrentMetadataChanged();
		}

		void setTrackReady() override
		{
			if(m_initialPositionMs)
			{
				const auto initialPositionMs = m_initialPositionMs.value();
				if(initialPositionMs > 0)
				{
					// NOLINTNEXTLINE(readability-magic-numbers)
					spLog(Log::Debug, this) << "Track ready, Start at " << (initialPositionMs / 1000) << " sec";
					seekAbsoluteMs(initialPositionMs);
				}

				m_initialPositionMs.reset();

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

		void setTrackFinished() override
		{
			next();
		}

		void buffering(const int progress) override
		{
			emit sigBuffering(progress);
		}

		void volumeUp() override
		{
			setVolume(volume() + VolumeDelta);
		}

		void volumeDown() override
		{
			setVolume(volume() - VolumeDelta);
		}

		void setVolume(int vol) override
		{
			vol = std::min(vol, 100); // NOLINT(readability-magic-numbers)
			vol = std::max(vol, 0);
			SetSetting(Set::Engine_Vol, vol);
			emit sigVolumeChanged(vol);
		}

		void setMute(const bool b) override
		{
			SetSetting(Set::Engine_Mute, b);
			emit sigMuteChanged(b);
		}

		void toggleMute() override
		{
			setMute(!GetSetting(Set::Engine_Mute));
		}

		void error(const QString& message) override
		{
			emit sigError(message);
		}

		void changeDuration(const MilliSeconds ms) override
		{
			m_currentTrack.setDurationMs(ms);
			emit sigDurationChangedMs();
		}

		void changeBitrate(const Bitrate br) override
		{
			m_currentTrack.setBitrate(br);
			emit sigBitrateChanged();
		}

		void shutdown() override
		{
			if(m_playstate == PlayState::Stopped)
			{
				SetSetting(Set::PL_LastTrack, -1);
				SetSetting(Set::Engine_CurTrackPos_s, 0);
			}

			else
			{
				SetSetting(Set::Engine_CurTrackPos_s, static_cast<int>(m_positionMs / 1000));
			}
		}

		void trackMetadataChanged()
		{
			auto* changeNotifier = dynamic_cast<Tagging::ChangeNotifier*>(sender());

			const auto& changedMetadata = changeNotifier->changedMetadata();
			const auto& it = Util::Algorithm::find(changedMetadata, [&](const auto& pair) {
				return pair.first.isEqual(m_currentTrack);
			});

			if(it != changedMetadata.end())
			{
				changeCurrentMetadata(it->second);
			}
		}

		void tracksDeleted()
		{
			auto* changeNotifier = dynamic_cast<Tagging::ChangeNotifier*>(sender());

			const auto& deletedTracks = changeNotifier->deletedMetadata();
			const auto containsCurrentTrack = Util::Algorithm::contains(deletedTracks, [&](const auto& track) {
				return m_currentTrack.filepath() == track.filepath();
			});

			if(containsCurrentTrack)
			{
				stop();
			}
		}

	private:
		void reset()
		{
			m_currentTrack = MetaData();
			m_ringBuffer.clear();
			m_currentTrackIndex = -1;
			m_positionMs = 0;
			m_trackPlaytimeMs = 0;
			m_initialPositionMs.reset();
			m_playstate = PlayState::Stopped;
		}

		MetaData m_currentTrack;
		RingBuffer<RingbufferSize> m_ringBuffer;
		int m_currentTrackIndex {-1};
		MilliSeconds m_positionMs {0};
		std::optional<MilliSeconds> m_initialPositionMs;
		MilliSeconds m_trackPlaytimeMs {0};
		PlayState m_playstate {PlayState::FirstStartup};
		NotificationHandler* m_notificationHandler;
};

PlayManager* PlayManager::create(NotificationHandler* notificationHandler, QObject* parent)
{
	return new PlayManagerImpl(notificationHandler, parent);
}
