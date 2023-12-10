/* PipelineProbes.cpp */

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

#include "Probing.h"

#include "Components/Engine/StreamRecorder/StreamRecorderData.h"
#include "Components/Engine/EngineUtils.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/StandardPaths.h"

using namespace PipelineExtensions;

GstPadProbeReturn
Probing::levelProbed(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
	Q_UNUSED(pad)
	Q_UNUSED(info)

	auto* b = static_cast<bool*>( user_data );
	if(*b)
	{
		return GST_PAD_PROBE_REMOVE;
	}

	else
	{
		return GST_PAD_PROBE_DROP;
	}
}

GstPadProbeReturn
Probing::spectrumProbed(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
	Q_UNUSED(pad)
	Q_UNUSED(info)

	auto* b = static_cast<bool*>( user_data );
	if(*b)
	{
		return GST_PAD_PROBE_REMOVE;
	}

	else
	{
		return GST_PAD_PROBE_DROP;
	}
}

GstPadProbeReturn
Probing::lameProbed(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
	Q_UNUSED(pad)
	Q_UNUSED(info)

	auto* b = static_cast<bool*>( user_data );
	if(*b)
	{
		return GST_PAD_PROBE_REMOVE;
	}

	else
	{
		return GST_PAD_PROBE_DROP;
	}
}

GstPadProbeReturn
Probing::pitchProbed(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
	Q_UNUSED(pad)
	Q_UNUSED(info)

	auto* b = static_cast<bool*>( user_data );
	if(*b)
	{
		return GST_PAD_PROBE_REMOVE;
	}

	else
	{
		return GST_PAD_PROBE_DROP;
	}
}

void Probing::handleProbe(bool* active, GstElement* queue, gulong* probe_id, GstPadProbeCallback callback)
{
	GstPad* pad = gst_element_get_static_pad(queue, "src");

	if(*active == true)
	{
		if(*probe_id > 0)
		{
			gst_pad_remove_probe(pad, *probe_id);
			*probe_id = 0;
		}
	}

	else if(*probe_id == 0)
	{
		*probe_id = gst_pad_add_probe(
			pad,
			(GstPadProbeType) (GST_PAD_PROBE_TYPE_BUFFER),
			callback,
			active, // userdata
			NULL
		);
	}

	if(pad != nullptr)
	{
		gst_object_unref(pad);
	}
}

void Probing::handleStreamRecorderProbe(StreamRecorder::Data* data, GstPadProbeCallback callback)
{
	GstPad* pad = gst_element_get_static_pad(data->queue, "src");

	if(data->probeId == 0)
	{
		data->busy = true;
		data->probeId = gst_pad_add_probe(
			pad,
			(GstPadProbeType) (GST_PAD_PROBE_TYPE_BUFFER),
			callback,
			data, // userdata
			NULL
		);

		gst_element_send_event(data->sink, gst_event_new_eos());
	}

	if(pad != nullptr)
	{
		gst_object_unref(pad);
	}
}

GstPadProbeReturn
Probing::streamRecorderProbed(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
	Q_UNUSED(pad)
	Q_UNUSED(info)

	auto* data = static_cast<StreamRecorder::Data*>(user_data);

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

		if(data->probeId > 0)
		{
			//gst_pad_remove_probe(pad, data->probe_id);
			data->probeId = 0;
		}

		Engine::Utils::setState(data->sink, GST_STATE_PLAYING);

		data->busy = false;
		return GST_PAD_PROBE_REMOVE;
	}

	else
	{
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
