/* DelayedPlayHandler.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef SAYONARA_ENGINE_DELAYED_PLAYABLE_H
#define SAYONARA_ENGINE_DELAYED_PLAYABLE_H

#include "PipelineInterfaces.h"
#include "Utils/typedefs.h"

#include <memory>
#include <gst/gst.h>

namespace PipelineExtensions
{
	class DelayedPlaybackInvoker
	{
		public:
			virtual ~DelayedPlaybackInvoker();

			virtual void playIn(MilliSeconds ms) = 0;
			virtual void abortDelayedPlaying() = 0;
	};

	std::shared_ptr<DelayedPlaybackInvoker>
	createDelayedPlaybackInvoker(PlaystateController* pipeline);
}

#endif // SAYONARA_ENGINE_DELAYED_PLAYABLE_H
