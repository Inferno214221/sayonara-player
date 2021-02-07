/* PlayManager.h */

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

#ifndef PLAY_MANAGER_H
#define PLAY_MANAGER_H

#include "Utils/Pimpl.h"
#include "Utils/Singleton.h"
#include "Utils/typedefs.h"

#include <QObject>

/**
 * @brief Global handler for current playback state (Singleton)
 * @ingroup Components
 */
class PlayManager :
	public QObject
{
	Q_OBJECT

	signals:

		/**
		 * @brief emitted when a streamed track has finished
		 * @param old_md the last played track
		 */
		void sigStreamFinished(const MetaData& old_md);

		/**
		 * @brief emitted, when PlayState was changed
		 */
		void sigPlaystateChanged(PlayState);

		/**
		 * @brief next track was triggered
		 */
		void sigNext();

		/**
		 * @brief This signal is sent when the playstate changed
		 * from stopped to play
		 */
		void sigWakeup();

		/**
		 * @brief previous track was triggered
		 */
		void sigPrevious();

		/**
		 * @brief stop was triggered
		 */
		void sigStopped();

		/**
		 * @brief relative seeking was triggered
		 * @param percent relative position in track
		 */
		void sigSeekedRelative(double percent);

		/**
		 * @brief relative seeking was triggered
		 * @param ms relative position to current position in milliseconds
		 */
		void sigSeekedRelativeMs(MilliSeconds ms);

		/**
		 * @brief absolute seeking was triggered
		 * @param ms absolute position in milliseconds
		 */
		void sigSeekedAbsoluteMs(MilliSeconds ms);

		/**
		 * @brief position in track has changed
		 * @param ms absolute position in milliseconds
		 */
		void sigPositionChangedMs(MilliSeconds ms);

		/**
		 * @brief track has changed
		 * @param md new MetaData
		 */
		void sigCurrentTrackChanged(const MetaData& md);

		void sigCurrentMetadataChanged();

		/**
		 * @brief track has changed
		 * @param idx index in playlist
		 */
		void sigTrackIndexChanged(int idx);

		/**
		 * @brief duration of track has changed
		 * @param ms duration of track in milliseconds
		 */
		void sigDurationChangedMs();

		void sigBitrateChanged();

		/**
		 * @brief playlist has finished
		 */
		void sigPlaylistFinished();

		/**
		 * @brief recording is requested
		 * @param b
		 *  true, when a new recording session should begin,
		 *  false if a recording session should stop
		 */
		void sigRecording(bool b);

		/**
		 * @brief emitted when currently in buffering state
		 * @param b true if buffering, false else
		 */
		void sigBuffering(int b);

		/**
		 * @brief emitted when volume has changed
		 * @param vol value between 0 and 100
		 */
		void sigVolumeChanged(int vol);

		/**
		 * @brief emitted when mute state has changed
		 * @param b true if muted, false else
		 */
		void sigMuteChanged(bool b);

		void sigError(const QString& message);

	public slots:
		/**
		 * @brief Start playing if there's a track
		 */
		virtual void play() = 0;

		/**
		 * @brief Emit wake up signal after stopping state
		 */
		virtual void wakeUp() = 0;

		/**
		 * @brief toggle play/pause
		 */
		virtual void playPause() = 0;

		/**
		 * @brief pause track, if currently playing
		 */
		virtual void pause() = 0;

		/**
		 * @brief change to previous track
		 */
		virtual void previous() = 0;

		/**
		 * @brief change to next track
		 */
		virtual void next() = 0;

		/**
		 * @brief stop playback
		 */
		virtual void stop() = 0;

		/**
		 * @brief request recording (see also sig_record(bool b))
		 * @param b
		 *  true, when a new recording session should begin,
		 *  false if a recording session should stop
		 */
		virtual void record(bool b) = 0;

		/**
		 * @brief seek relative
		 * @param percent relative position within track
		 */
		virtual void seekRelative(double percent) = 0;

		/**
		 * @brief seek absolute
		 * @param ms absolute position in milliseconds
		 */
		virtual void seekAbsoluteMs(MilliSeconds ms) = 0;

		/**
		 * @brief seekRelativeMs
		 * @param ms relative position to current position in milliseconds
		 */
		virtual void seekRelativeMs(MilliSeconds ms) = 0;

		/**
		 * @brief set current position of track
		 * This method does not seek.
		 * Just tells the playmanager where the current position is
		 * @param ms position in milliseconds.
		 */
		virtual void setCurrentPositionMs(MilliSeconds ms) = 0;

		/**
		 * @brief change current track
		 * @param md new MetaData object
		 */
		virtual void changeCurrentTrack(const MetaData& md, int trackIdx) = 0;

		/**
		 * @brief change_track
		 * @param md
		 */
		virtual void changeCurrentMetadata(const MetaData& md) = 0;

		/**
		 * @brief notify, that track is ready for playback
		 */
		virtual void setTrackReady() = 0;
		virtual void setTrackFinished() = 0;

		/**
		 * @brief notifiy, that track is in buffering state currently
		 * @param progress
		 */
		virtual void buffering(int progress) = 0;

		/**
		 * @brief increase volume by 5
		 */
		virtual void volumeUp() = 0;

		/**
		 * @brief decrease volume by 5
		 */
		virtual void volumeDown() = 0;

		/**
		 * @brief set volume
		 * @param vol value between [0,100], will be cropped if not within boundaries
		 */
		virtual void setVolume(int vol) = 0;

		/**
		 * @brief mute/unmute
		 * @param b
		 */
		virtual void setMute(bool b) = 0;

		/**
		 * @brief If already muted, then unmute. If unmuted, then mute it
		 */
		virtual void toggleMute() = 0;

		/**
		 * @brief Change the duration. This is usually called when
		 * the Engine sends a duration changed signal. You should
		 * not use this
		 * @param ms
		 */
		virtual void changeDuration(MilliSeconds ms) = 0;

		virtual void changeBitrate(Bitrate br) = 0;

		/**
		 * @brief Some playback error occured
		 * @param message
		 */
		virtual void error(const QString& message) = 0;

	public:
		PlayManager() = default;
		virtual ~PlayManager() = default;

		/**
		 * @brief get current play state
		 * @return PlayState enum
		 */
		virtual PlayState playstate() const = 0;

		/**
		 * @brief get current position in milliseconds
		 * @return current position in milliseconds
		 */
		virtual MilliSeconds currentPositionMs() const = 0;

		virtual MilliSeconds currentTrackPlaytimeMs() const = 0;

		/**
		 * @brief get position in milliseconds where track will start
		 * @return position in milliseconds where track will start
		 */
		virtual MilliSeconds initialPositionMs() const = 0;

		/**
		 * @brief get duration of track
		 * @return duration in milliseconds
		 */
		virtual MilliSeconds durationMs() const = 0;

		virtual Bitrate bitrate() const = 0;

		/**
		 * @brief get current track
		 * @return MetaData object of current track
		 */
		virtual const MetaData& currentTrack() const = 0;

		/**
		 * @brief get current volume
		 * @return value between 0 and 100
		 */
		virtual int volume() const = 0;

		/**
		 * @brief query mute status
		 * @return true if muted, false else
		 */
		virtual bool isMuted() const = 0;

		/**
		 * @brief Shutdown the computer
		 */
		virtual void shutdown() = 0;
};

#endif


