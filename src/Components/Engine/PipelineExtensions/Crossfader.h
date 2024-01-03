/* CrossFader.h */

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

/* CrossFader.h */

#ifndef SAYONARA_ENGINE_CROSSFADER_H
#define SAYONARA_ENGINE_CROSSFADER_H

#include "Utils/Pimpl.h"
#include "PipelineInterfaces.h"
#include <QThread>

#include <gst/gst.h>

namespace PipelineExtensions
{
	class Crossfader
	{
		public:
			enum class FadeMode :
				unsigned char
			{
				NoFading = 0,
				FadeIn,
				FadeOut
			};

			virtual ~Crossfader();

			virtual void fadeIn() = 0;
			virtual void fadeOut() = 0;
			virtual void abortFading() = 0;

			[[nodiscard]] static MilliSeconds fadingTimeMs();
	};

	std::shared_ptr<Crossfader>
	createCrossfader(PlaystateController* playStateController, VolumeController* volumeController);
}

#endif // SAYONARA_ENGINE_CROSSFADER_H
