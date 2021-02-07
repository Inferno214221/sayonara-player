/* RawSoundReceiver.h */

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

#ifndef AUDIO_DATA_RECEIVER_INTERFACES_H
#define AUDIO_DATA_RECEIVER_INTERFACES_H

#include "Utils/typedefs.h"

#include <vector>

class QByteArray;

namespace Engine
{
	/**
	 * @brief The LevelReceiver class
	 * @ingroup EngineInterfaces
	 */
	class LevelReceiver
	{
		public:
			LevelReceiver() = default;
			virtual ~LevelReceiver() = default;

			virtual void setLevel(float left, float right) = 0;
			virtual bool isActive() const = 0;
	};

	/**
	 * @brief The SpectrumReceiver class
	 * @ingroup EngineInterfaces
	 */
	class SpectrumReceiver
	{
		public:
			SpectrumReceiver() = default;
			virtual ~SpectrumReceiver() = default;

			virtual void setSpectrum(const std::vector<float>& spectrum) = 0;
			virtual bool isActive() const = 0;
	};

	/**
	 * @brief The RawSoundReceiver interface
	 * @ingroup EngineInterfaces
	 */
	class RawAudioDataReceiver
	{
		public:
			RawAudioDataReceiver() = default;
			virtual ~RawAudioDataReceiver() = default;

			/**
			 * @brief triggered when new audio data is available, has to be reimplentend
			 * @param data audio data
			 * @param n_bytes array size
			 */
			virtual void writeAudioData(const QByteArray& data) = 0;
	};
}

#endif // AUDIO_DATA_RECEIVER_INTERFACES_H
