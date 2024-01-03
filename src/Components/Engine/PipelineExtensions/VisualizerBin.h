/* Visualizer.h */

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

#ifndef SAYONARA_ENGINE_VISUALIZER_H
#define SAYONARA_ENGINE_VISUALIZER_H

#include <gst/gst.h>
#include <memory>

namespace PipelineExtensions
{
	class VisualizerBin
	{
		public:
			virtual ~VisualizerBin();

			virtual bool setEnabled(bool levelEnabled, bool spectrumEnabled) = 0;

			[[nodiscard]] virtual bool isLevelEnabled() const = 0;
			[[nodiscard]] virtual bool isSpectrumEnabled() const = 0;
	};

	std::shared_ptr<VisualizerBin> createVisualizerBin(GstElement* pipeline, GstElement* tee);
}

#endif // SAYONARA_ENGINE_VISUALIZER_H
