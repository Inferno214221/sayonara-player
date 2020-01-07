/* PipelineProbes.h */

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

#ifndef PIPELINEPROBES_H
#define PIPELINEPROBES_H

#include <gst/gst.h>

namespace StreamRecorder
{
	struct Data;
}

namespace PipelineExtensions
{
	/**
	 * @ingroup EngineHelper
	 */
	namespace Probing
	{

		/**
		 * @brief This is the main interface for the outside world
		 * @ingroup EngineHelper
		 * @param active
		 * @param queue
		 * @param probe_id
		 * @param callback
		 */
		void handle_probe(bool* active, GstElement* queue, gulong* probe_id, GstPadProbeCallback callback);


		/**
		 * @brief level_probed
		 * @ingroup EngineHelper
		 * @param pad
		 * @param info
		 * @param user_data
		 * @return
		 */
		GstPadProbeReturn
		level_probed(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

		/**
		 * @brief spectrum_probed
		 * @ingroup EngineHelper
		 * @param pad
		 * @param info
		 * @param user_data
		 * @return
		 */
		GstPadProbeReturn
		spectrum_probed(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

		/**
		 * @brief lame_probed
		 * @ingroup EngineHelper
		 * @param pad
		 * @param info
		 * @param user_data
		 * @return
		 */
		GstPadProbeReturn
		lame_probed(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

		/**
		 * @brief pitch_probed
		 * @ingroup EngineHelper
		 * @param pad
		 * @param info
		 * @param user_data
		 * @return
		 */
		GstPadProbeReturn
		pitch_probed(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);


		/**
		 * @brief stream_recorder_probed
		 * @ingroup EngineHelper
		 * @param pad
		 * @param info
		 * @param user_data
		 * @return
		 */
		GstPadProbeReturn
		stream_recorder_probed(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

		/**
		 * @brief handle_stream_recorder_probe
		 * @ingroup EngineHelper
		 * @param data
		 * @param callback
		 */
		void handle_stream_recorder_probe(StreamRecorder::Data* data, GstPadProbeCallback callback);
	}
}

#endif // PIPELINEPROBES_H
