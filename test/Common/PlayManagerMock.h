/* ${CLASS_NAME}.h */
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
#ifndef SAYONARA_PLAYER_PLAYMANAGERMOCK_H
#define SAYONARA_PLAYER_PLAYMANAGERMOCK_H

#include "Components/PlayManager/PlayManager.h"
#include "Utils/MetaData/MetaData.h"

class PlayManagerMock : public PlayManager
{
	MetaData m_metadata;

	public:
		PlayManagerMock() : PlayManager(nullptr) {}

		void play() override {}

		void wakeUp() override {}

		void playPause() override {}

		void pause() override {}

		void previous() override {}

		void next() override {}

		void stop() override {}

		void record(bool) override {}

		void seekRelative(double) override {}

		void seekAbsoluteMs(MilliSeconds) override {}

		void seekRelativeMs(MilliSeconds) override {}

		void setCurrentPositionMs(MilliSeconds) override {}

		void changeCurrentTrack(const MetaData&, int) override {}

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

		PlayState playstate() const override
		{
			return PlayState::Playing;
		}

		MilliSeconds currentPositionMs() const override
		{
			return 0;
		}

		MilliSeconds currentTrackPlaytimeMs() const override
		{
			return 0;
		}

		MilliSeconds initialPositionMs() const override
		{
			return 0;
		}

		MilliSeconds durationMs() const override
		{
			return 0;
		}

		Bitrate bitrate() const override
		{
			return 0;
		}

		const MetaData& currentTrack() const override
		{
			return m_metadata;
		}

		int volume() const override
		{
			return 0;
		}

		bool isMuted() const override
		{
			return false;
		}

		void shutdown() override {}
};

#endif //SAYONARA_PLAYER_PLAYMANAGERMOCK_H
