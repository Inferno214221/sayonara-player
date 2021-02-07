/* AudioDataProvider.h */
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
#ifndef SAYONARA_PLAYER_AUDIODATAPROVIDER_H
#define SAYONARA_PLAYER_AUDIODATAPROVIDER_H

#include <vector>

namespace Engine
{
	class LevelReceiver;
	class SpectrumReceiver;
}

class LevelDataProvider
{
	public:
		virtual ~LevelDataProvider() = default;

		virtual void setLevelData(float left, float right)=0;

		virtual void registerLevelReceiver(Engine::LevelReceiver* levelReceiver)=0;
		virtual void unregisterLevelReceiver(Engine::LevelReceiver* levelReceiver)=0;

		virtual void levelActiveChanged(bool b)=0;
};

class SpectrumDataProvider
{
	public:
		virtual ~SpectrumDataProvider() = default;

		virtual void setSpectrumData(const std::vector<float>& spectrum)=0;

		virtual void registerSpectrumReceiver(Engine::SpectrumReceiver* spectrumReceiver)=0;
		virtual void unregisterSpectrumReceiver(Engine::SpectrumReceiver* spectrumReceiver)=0;

		virtual void spectrumActiveChanged(bool b)=0;
};

#endif //SAYONARA_PLAYER_AUDIODATAPROVIDER_H
