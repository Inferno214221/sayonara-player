/* SpeedHandler.cpp */

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

#include "SpeedHandler.h"
#include "../Callbacks/EngineUtils.h"
#include "Utils/Settings/Settings.h"

#include <gst/gst.h>

using Pipeline::SpeedHandler;

struct SpeedHandler::Private
{
	GstElement* pitch=nullptr;

	Private()
	{

	}
};

SpeedHandler::SpeedHandler()
{
	m = Pimpl::make<Private>();
}

SpeedHandler::~SpeedHandler() {}

void SpeedHandler::set_speed(float speed, double pitch, bool preserve_pitch)
{
	if(!GetSetting(Set::Engine_SpeedActive)) {
		return;
	}

	if(!m->pitch){
		return;
	}

	if(preserve_pitch)
	{
		Engine::Utils::set_values(m->pitch,
					 "tempo", speed,
					 "rate", 1.0,
					 "pitch", pitch);
	}

	else
	{
		Engine::Utils::set_values(m->pitch,
					 "tempo", 1.0,
					 "rate", speed,
					 "pitch", pitch);
	}
}

GstElement* SpeedHandler::element() const
{
	return m->pitch;
}
