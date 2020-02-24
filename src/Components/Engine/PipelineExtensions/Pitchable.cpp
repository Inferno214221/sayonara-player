/* SpeedHandler.cpp */

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

#include "Pitchable.h"
#include "Components/Engine/EngineUtils.h"
#include "Utils/Settings/Settings.h"

using namespace PipelineExtensions;

Pitchable::~Pitchable() = default;

void Pitchable::setSpeed(float speed, double pitch, bool preservePitch)
{
	if(!GetSetting(Set::Engine_SpeedActive)) {
		return;
	}

	GstElement* pitchElement = this->pitchElement();
	if(!pitchElement) {
		return;
	}

	if(preservePitch)
	{
		Engine::Utils::setValues(pitchElement,
					 "tempo", speed,
					 "rate", 1.0,
					 "pitch", pitch);
	}

	else
	{
		Engine::Utils::setValues(pitchElement,
					 "tempo", 1.0,
					 "rate", speed,
					 "pitch", pitch);
	}
}
