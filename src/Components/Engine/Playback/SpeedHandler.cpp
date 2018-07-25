/* SpeedHandler.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "Utils/Settings/Settings.h"

#include <gst/gst.h>

using Pipeline::SpeedHandler;

SpeedHandler::SpeedHandler() {}

SpeedHandler::~SpeedHandler() {}

void SpeedHandler::set_speed(float speed, double pitch, bool preserve_pitch)
{
	Settings* s = Settings::instance();
	if(!s->get<Set::Engine_SpeedActive>())
	{
		return;
	}

	GstElement* pitch_element = get_pitch_element();

	if(!pitch_element){
		return;
	}

	if(preserve_pitch){
		g_object_set(pitch_element,
					 "tempo", speed,
					 "rate", 1.0,
					 "pitch", pitch,
					 nullptr);
	}

	else{
		g_object_set(pitch_element,
					 "tempo", 1.0,
					 "rate", speed,
					 "pitch", pitch,
					 nullptr);
	}
}
