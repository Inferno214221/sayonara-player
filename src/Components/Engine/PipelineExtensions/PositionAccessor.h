/* SeekHandler.h */

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

#ifndef SEEKHANDLER_H
#define SEEKHANDLER_H

#include "Components/Engine/gstfwd.h"
#include "Utils/typedefs.h"

#include <memory>

namespace PipelineExtensions
{
	class PositionAccessor
	{
		public:
			virtual ~PositionAccessor();

			virtual void seekRelative(double percent, MilliSeconds duration) = 0;
			virtual void seekAbsoluteMs(MilliSeconds ms) = 0;
			virtual void seekNearestMs(MilliSeconds ms) = 0;

			[[nodiscard]] virtual MilliSeconds timeToGo() const = 0;
			[[nodiscard]] virtual MilliSeconds positionMs() const = 0;
			[[nodiscard]] virtual MilliSeconds durationMs() const = 0;
	};

	std::shared_ptr<PositionAccessor> createPositionAccessor(GstElement* positionElement);
}

#endif // SEEKHANDLER_H
