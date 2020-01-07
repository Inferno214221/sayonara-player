/* SeekHandler.h */

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



#ifndef SEEKHANDLER_H
#define SEEKHANDLER_H

#include "Components/Engine/gstfwd.h"
#include "Utils/typedefs.h"
#include "Utils/Pimpl.h"

namespace PipelineExtensions
{

	/**
	 * @brief The Seeker class
	 * @ingroup EngineInterfaces
	 */
	class Seeker
	{
		PIMPL(Seeker)

		public:
			Seeker(GstElement* source);
			virtual ~Seeker();

			NanoSeconds seek_rel(double percent, NanoSeconds ref_ns);
			NanoSeconds seek_abs(NanoSeconds ns);
			NanoSeconds seek_nearest(NanoSeconds ns);

			NanoSeconds seek_rel_ms(double percent, MilliSeconds ref_ns);
			NanoSeconds seek_abs_ms(MilliSeconds ns);
			NanoSeconds seek_nearest_ms(MilliSeconds ns);

			void set_source(GstElement* source);
	};
}

#endif // SEEKHANDLER_H
