/* SpeedHandler.h */

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



#ifndef SPEEDHANDLER_H
#define SPEEDHANDLER_H

#include "Components/Engine/gstfwd.h"
#include "Utils/Pimpl.h"

namespace PipelineExtensions
{
	class SpeedHandler
	{
		PIMPL(SpeedHandler)

		public:
			SpeedHandler();
			virtual ~SpeedHandler();

			void set_speed(float speed, double pitch, bool preserve_pitch);

			GstElement* element() const;
	};
}

#endif // SPEEDHANDLER_H
