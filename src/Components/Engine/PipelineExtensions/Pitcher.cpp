/* SpeedHandler.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "Pitcher.h"
#include "Components/Engine/EngineUtils.h"
#include "Utils/Settings/Settings.h"

using namespace PipelineExtensions;

struct Pitcher::Private
{
	GstElement* pitch=nullptr;

	Private()
	{
		Engine::Utils::create_element(&pitch, "pitch");
	}
};

Pitcher::Pitcher()
{
	m = Pimpl::make<Private>();
}

Pitcher::~Pitcher() {}

void Pitcher::set_speed(float speed, double pitch, bool preserve_pitch)
{
	if(!GetSetting(Set::Engine_SpeedActive)) {
		return;
	}

	if(!m->pitch) {
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

GstElement* Pitcher::element() const
{
	return m->pitch;
}
