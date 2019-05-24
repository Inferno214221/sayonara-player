/* SoundOutReceiver.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef SOUNDOUTRECEIVER_H
#define SOUNDOUTRECEIVER_H

#include <vector>

using SpectrumList=std::vector<float>;

/**
 * @brief The LevelReceiver class
 * @ingroup EngineInterfaces
 */
class LevelReceiver
{
	public:
	virtual void set_level(float left, float right)=0;
	virtual bool is_active() const=0;

	LevelReceiver();
	virtual ~LevelReceiver();
};

/**
 * @brief The SpectrumReceiver class
 * @ingroup EngineInterfaces
 */
class SpectrumReceiver
{
public:
	virtual void set_spectrum(const SpectrumList& spectrum)=0;
	virtual bool is_active() const=0;

	SpectrumReceiver();
	virtual ~SpectrumReceiver();
};

#endif // LEVELRECEIVER_H

