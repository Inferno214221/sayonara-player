/* PipelineCallbacks.cpp */

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

#include "PipelineCallbacks.h"
#include "EngineUtils.h"

#include "Utils/Logger/Logger.h"
#include "Utils/WebAccess/Proxy.h"
#include "Components/Engine/Playback/PlaybackPipeline.h"

#include <gst/app/gstappsink.h>
#include <gst/base/gstbasesrc.h>

const char* ClassPipelineCallbacks="PipelineCallbacks";

using Pipeline::Playback;
using namespace Pipeline;

gboolean Callbacks::position_changed(gpointer data)
{
	GstState state;
	Playback* pipeline;

	pipeline = static_cast<Playback*>(data);
	if(!pipeline){
		return false;
	}

	state = pipeline->get_state();

	if( state != GST_STATE_PLAYING &&
		state != GST_STATE_PAUSED &&
		state != GST_STATE_READY)
	{
		return true;
	}

	pipeline->refresh_position();
	pipeline->check_about_to_finish();

	return true;
}

// dynamic linking, important for decodebin
void Callbacks::decodebin_ready(GstElement* source, GstPad* new_src_pad, gpointer data)
{
	gchar* element_name = gst_element_get_name(source);
	sp_log(Log::Develop, "Callback") << "Source: " << element_name;
	g_free(element_name);

	GstElement* element = static_cast<GstElement*>(data);
	if(!element){
		return;
	}

	GstPad*	sink_pad = gst_element_get_static_pad(element, "sink");
	if(!sink_pad){
		return;
	}

	if(gst_pad_is_linked(sink_pad))
	{
		gst_object_unref(sink_pad);
		return;
	}

	GstPadLinkReturn pad_link_return = gst_pad_link(new_src_pad, sink_pad);

	if(pad_link_return != GST_PAD_LINK_OK)
	{
		sp_log(Log::Error, ClassPipelineCallbacks) << "Dynamic pad linking: Cannot link pads";

		switch(pad_link_return){
			case GST_PAD_LINK_WRONG_HIERARCHY:
				sp_log(Log::Error, ClassPipelineCallbacks) << "Cause: Wrong hierarchy";
				break;
			case GST_PAD_LINK_WAS_LINKED:
				sp_log(Log::Error, ClassPipelineCallbacks) << "Cause: Pad was already linked";
				break;
			case GST_PAD_LINK_WRONG_DIRECTION:
				sp_log(Log::Error, ClassPipelineCallbacks) << "Cause: Pads have wrong direction";
				break;
			case GST_PAD_LINK_NOFORMAT:
				sp_log(Log::Error, ClassPipelineCallbacks) << "Cause: Pads have incompatible format";
				break;
			case GST_PAD_LINK_NOSCHED:
				sp_log(Log::Error, ClassPipelineCallbacks) << "Cause: Pads cannot cooperate scheduling";
				break;
			case GST_PAD_LINK_REFUSED:
			default:
				sp_log(Log::Error, ClassPipelineCallbacks) << "Cause: Refused because of different reason";
				break;
		}
	}

	else {
		sp_log(Log::Debug, "PipelineCallbacks") << "Successfully linked " << gst_element_get_name(source) << " with " << gst_element_get_name(element);
	}

	gst_object_unref(sink_pad);
}


#define TCP_BUFFER_SIZE 16384
GstFlowReturn Callbacks::new_buffer(GstElement *sink, gpointer p)
{
	static uchar data[TCP_BUFFER_SIZE];

	Playback* pipeline = static_cast<Playback*>(p);
	if(!pipeline){
		return GST_FLOW_OK;
	}

	GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
	if(!sample) {
		return GST_FLOW_OK;
	}

	GstBuffer* buffer = gst_sample_get_buffer(sample);
	if(!buffer) {
		gst_sample_unref(sample);
		return GST_FLOW_OK;
	}

	gsize size = gst_buffer_get_size(buffer);
	gsize size_new = gst_buffer_extract(buffer, 0, data, size);
	pipeline->set_data(data, size_new);

	gst_sample_unref(sample);

	return GST_FLOW_OK;
}


static bool is_source_soup(GstElement* source)
{
	GstElementFactory* fac = gst_element_get_factory(source);
	GType type = gst_element_factory_get_element_type(fac);

	const gchar* name = g_type_name(type);
	QString src_type(name);

	return (src_type.compare("gstsouphttpsrc", Qt::CaseInsensitive) == 0);
}


void Callbacks::source_ready(GstURIDecodeBin* bin, GstElement* source, gpointer data)
{
	Q_UNUSED(bin);
	Q_UNUSED(data);

	sp_log(Log::Develop, "Engine Callback") << "Source ready: is soup? " << is_source_soup(source);
	gst_base_src_set_dynamic_size(GST_BASE_SRC(source), false);

	if(is_source_soup(source))
	{
		Proxy* proxy = Proxy::instance();
		if(proxy->active())
		{
			sp_log(Log::Develop, "Engine Callback") << "Will use proxy: " << proxy->full_url();

			if(proxy->has_username())
			{
				sp_log(Log::Develop, "Engine Callback") << "Will use proxy username: " << proxy->username();

				Engine::Utils::set_values(source,
						"proxy-id", proxy->username().toLocal8Bit().data(),
						"proxy-pw", proxy->password().toLocal8Bit().data());
			}
		}
	}
}
