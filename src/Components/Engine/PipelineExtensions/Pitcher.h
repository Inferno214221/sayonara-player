/* SpeedHandler.h */

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



#ifndef SPEEDHANDLER_H
#define SPEEDHANDLER_H

#include "Components/Engine/gstfwd.h"
#include "Utils/Pimpl.h"

namespace PipelineExtensions
{

	/**
	 * @brief The Pitcher class
	 * @ingroup EngineInterfaces
	 */
	class Pitcher
	{
		PIMPL(Pitcher)

		public:
			Pitcher();
			virtual ~Pitcher();

			void setSpeed(float speed, double pitch, bool preservePitch);

			GstElement* element() const;
	};
}

#endif // SPEEDHANDLER_H
