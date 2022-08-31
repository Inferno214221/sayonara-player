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

#include <QObject>

class PlayManager :
	public QObject
{
	Q_OBJECT

	signals:
		void sigStreamFinished(const MetaData& oldTrack);
		void sigPlaystateChanged(PlayState playState);
		void sigNext();
		void sigWakeup();
		void sigPrevious();
		void sigStopped();
		void sigSeekedRelative(double percent);
		void sigSeekedRelativeMs(MilliSeconds ms);
		void sigSeekedAbsoluteMs(MilliSeconds ms);
		void sigPositionChangedMs(MilliSeconds ms);
		void sigCurrentTrackChanged(const MetaData& track);
		void sigCurrentMetadataChanged();
		void sigTrackIndexChanged(int idx);
		void sigDurationChangedMs();
		void sigBitrateChanged();
		void sigPlaylistFinished();
		void sigRecording(bool b);
		void sigBuffering(int b);
		void sigVolumeChanged(int vol);
		void sigMuteChanged(bool b);
		void sigError(const QString& message);

	public:
		~PlayManager() override = default;

		[[nodiscard]] virtual PlayState playstate() const = 0;
		[[nodiscard]] virtual MilliSeconds currentPositionMs() const = 0;
		[[nodiscard]] virtual MilliSeconds currentTrackPlaytimeMs() const = 0;
		[[nodiscard]] virtual MilliSeconds initialPositionMs() const = 0;
		[[nodiscard]] virtual MilliSeconds durationMs() const = 0;
		[[nodiscard]] virtual Bitrate bitrate() const = 0;
		[[nodiscard]] virtual const MetaData& currentTrack() const = 0;
		[[nodiscard]] virtual int volume() const = 0;
		[[nodiscard]] virtual bool isMuted() const = 0;

	public slots: // NOLINT(readability-redundant-access-specifiers)
		virtual void play() = 0;
		virtual void wakeUp() = 0;
		virtual void playPause() = 0;
		virtual void pause() = 0;
		virtual void previous() = 0;
		virtual void next() = 0;
		virtual void stop() = 0;
		virtual void record(bool b) = 0;
		virtual void seekRelative(double percent) = 0;
		virtual void seekAbsoluteMs(MilliSeconds ms) = 0;
		virtual void seekRelativeMs(MilliSeconds ms) = 0;
		virtual void setCurrentPositionMs(MilliSeconds ms) = 0;
		virtual void changeCurrentTrack(const MetaData& track, int trackIdx) = 0;
		virtual void changeCurrentMetadata(const MetaData& track) = 0;
		virtual void setTrackReady() = 0;
		virtual void setTrackFinished() = 0;
		virtual void buffering(int progress) = 0;
		virtual void volumeUp() = 0;
		virtual void volumeDown() = 0;
		virtual void setVolume(int vol) = 0;
		virtual void setMute(bool b) = 0;
		virtual void toggleMute() = 0;
		virtual void changeDuration(MilliSeconds ms) = 0;
		virtual void changeBitrate(Bitrate br) = 0;
		virtual void error(const QString& message) = 0;
		virtual void shutdown() = 0;

	private:
		static PlayManager* create();
};

#endif
