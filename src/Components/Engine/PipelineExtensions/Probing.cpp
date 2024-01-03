/* PipelineProbes.cpp */

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

#include "Probing.h"

#include "Components/Engine/StreamRecorder/StreamRecorderData.h"
#include "Components/Engine/EngineUtils.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/StandardPaths.h"

namespace PipelineExtensions::Probing
{
	void handleProbe(GenericProbingData* userData, GstPadProbeCallback callback)
	{
		auto pad = Engine::Utils::getStaticPad(userData->queue, Engine::Utils::src);
		if(userData->active)
		{
			if(userData->probeId > 0)
			{
				gst_pad_remove_probe(*pad, userData->probeId);
				userData->probeId = 0;
			}
		}

		else if(userData->probeId == 0)
		{
			userData->probeId = gst_pad_add_probe(*pad, GST_PAD_PROBE_TYPE_BUFFER, callback, userData, nullptr);
		}
	}

	void handleStreamRecorderProbe(StreamRecorder::Data* data, GstPadProbeCallback callback)
	{
		if(data->probeId == 0)
		{
			auto pad = Engine::Utils::getStaticPad(data->queue, Engine::Utils::src);

			data->busy = true;
			// NOLINTNEXTLINE(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
			data->probeId = gst_pad_add_probe(*pad, GST_PAD_PROBE_TYPE_BUFFER, callback, data, nullptr);

			gst_element_send_event(data->sink, gst_event_new_eos());
		}
	}

	GstPadProbeReturn streamRecorderProbed(GstPad* /*pad*/, GstPadProbeInfo* /*info*/, gpointer userData)
	{
		auto* data = static_cast<StreamRecorder::Data*>(userData);
		if(!data)
		{
			return GST_PAD_PROBE_DROP;
		}

		if(data->active)
		{
			spLog(Log::Develop, "PipelineProbes") << "set new filename streamrecorder: " << data->filename;

			Engine::Utils::setState(data->sink, GST_STATE_NULL);
			Engine::Utils::setValue(data->sink, "location", data->filename);

			data->isFilenameEmpty = false;
			data->probeId = std::min(0, data->probeId);

			Engine::Utils::setState(data->sink, GST_STATE_PLAYING);

			data->busy = false;

			return GST_PAD_PROBE_REMOVE;
		}

		if(!data->isFilenameEmpty)
		{
			Engine::Utils::setState(data->sink, GST_STATE_NULL);
			Engine::Utils::setValue(data->sink,
			                        "location",
			                        Util::tempPath("probing.mp3").toLocal8Bit().data());

			data->isFilenameEmpty = true;
		}

		data->busy = false;

		return GST_PAD_PROBE_DROP;
	}
}