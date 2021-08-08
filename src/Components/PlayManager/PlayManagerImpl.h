/* PlayManagerImpl.h */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_PLAYMANAGERIMPL_H
#define SAYONARA_PLAYER_PLAYMANAGERIMPL_H

#include "Interfaces/PlayManager.h"
#include "Utils/Pimpl.h"
#include "Utils/typedefs.h"

class PlayManagerImpl :
	public PlayManager
{
	Q_OBJECT
	PIMPL(PlayManagerImpl)

	public:
		PlayManagerImpl(QObject* parent);
		~PlayManagerImpl() override;

	protected:
		void play() override;
		void wakeUp() override;
		void playPause() override;
		void pause() override;
		void previous() override;
		void next() override;
		void stop() override;
		void record(bool b) override;
		void seekRelative(double percent) override;
		void seekAbsoluteMs(MilliSeconds ms) override;
		void seekRelativeMs(MilliSeconds ms) override;
		void setCurrentPositionMs(MilliSeconds ms) override;
		void changeCurrentTrack(const MetaData& track, int trackIdx) override;
		void changeCurrentMetadata(const MetaData& newMetadata) override;
		void setTrackReady() override;
		void setTrackFinished() override;
		void buffering(int progress) override;
		void volumeUp() override;
		void volumeDown() override;
		void setVolume(int vol) override;
		void setMute(bool b) override;
		void toggleMute() override;
		void changeDuration(MilliSeconds ms) override;
		void changeBitrate(Bitrate br) override;
		void error(const QString& message) override;

		PlayState playstate() const override;
		MilliSeconds currentPositionMs() const override;
		MilliSeconds currentTrackPlaytimeMs() const override;
		MilliSeconds initialPositionMs() const override;
		MilliSeconds durationMs() const override;
		Bitrate bitrate() const override;
		const MetaData& currentTrack() const override;
		int volume() const override;
		bool isMuted() const override;
		void shutdown() override;

	private slots:
		void trackMetadataChanged();
		void tracksDeleted();
};

#endif //SAYONARA_PLAYER_PLAYMANAGERIMPL_H
