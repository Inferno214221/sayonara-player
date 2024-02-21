/* ${CLASS_NAME}.h */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_PLAYMANAGERMOCK_H
#define SAYONARA_PLAYER_PLAYMANAGERMOCK_H

#include "Components/PlayManager/PlayManager.h"
#include "Utils/MetaData/MetaData.h"

class PlayManagerMock :
	public PlayManager
{
	public:
		PlayManagerMock() :
			PlayManager(nullptr) {}

		void play() override { m_playstate = PlayState::Playing; }

		void wakeUp() override {}

		void playPause() override
		{
			m_playstate = (m_playstate == PlayState::Playing)
			              ? PlayState::Paused
			              : PlayState::Playing;
		}

		void pause() override { m_playstate = PlayState::Paused; }

		void previous() override {}

		void next() override {}

		void stop() override { m_playstate = PlayState::Stopped; }

		void record(bool) override {}

		void seekRelative(double) override {}

		void seekAbsoluteMs(MilliSeconds) override {}

		void seekRelativeMs(MilliSeconds) override {}

		void setCurrentPositionMs(MilliSeconds) override {}

		void changeCurrentTrack(const MetaData& track, int) override { m_metadata = track; }

		void changeCurrentMetadata(const MetaData& md) override { m_metadata = md; }

		void setTrackReady() override {}

		void setTrackFinished() override {}

		void buffering(int) override {}

		void volumeUp() override {}

		void volumeDown() override {}

		void setVolume(int) override {}

		void setMute(bool) override {}

		void toggleMute() override {}

		void changeDuration(MilliSeconds) override {}

		void changeBitrate(Bitrate) override {}

		void error(const QString&) override {}

		[[nodiscard]] PlayState playstate() const override { return m_playstate; }

		void setPlaystate(const PlayState playState) { m_playstate = playState; }

		[[nodiscard]] MilliSeconds currentPositionMs() const override { return 0; }

		[[nodiscard]] MilliSeconds currentTrackPlaytimeMs() const override { return 0; }

		[[nodiscard]] MilliSeconds initialPositionMs() const override { return 0; }

		[[nodiscard]] MilliSeconds durationMs() const override { return 0; }

		[[nodiscard]] Bitrate bitrate() const override { return 0; }

		[[nodiscard]] const MetaData& currentTrack() const override { return m_metadata; }

		[[nodiscard]] int volume() const override { return 0; }

		[[nodiscard]] bool isMuted() const override { return false; }

		void shutdown() override {}

	private:
		MetaData m_metadata;
		PlayState m_playstate {PlayState::FirstStartup};
};

#endif //SAYONARA_PLAYER_PLAYMANAGERMOCK_H
