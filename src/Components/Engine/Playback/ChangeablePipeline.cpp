/* ChangeablePipeline.cpp */

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

#include "ChangeablePipeline.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

using Pipeline::Changeable;

const int SleepInterval = 50;

struct ProbeData
{
	GstElement* first_element=nullptr;
	GstElement* second_element=nullptr;
	GstElement* element_of_interest=nullptr;
	GstElement* pipeline=nullptr;
	GstState old_state;
	bool done;

	ProbeData()
	{
		old_state = GST_STATE_NULL;
		done = false;
	}
};

Changeable::Changeable() {}

Changeable::~Changeable() {}


static GstPadProbeReturn
src_blocked_add(GstPad* pad, GstPadProbeInfo* info, gpointer data)
{
	ProbeData* probe_data = (ProbeData*) data;

	gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));

	gst_element_set_state(probe_data->first_element, GST_STATE_NULL);
	gst_element_set_state(probe_data->element_of_interest, GST_STATE_NULL);

	gst_bin_add(GST_BIN(probe_data->pipeline), probe_data->element_of_interest);

	if(probe_data->second_element){
		gst_element_unlink(probe_data->first_element, probe_data->second_element);
	}

	gst_element_link(probe_data->first_element, probe_data->element_of_interest);
	if(probe_data->second_element){
		gst_element_link(probe_data->element_of_interest, probe_data->second_element);
	}

	gst_element_set_state(probe_data->element_of_interest, probe_data->old_state);
	gst_element_set_state(probe_data->first_element, probe_data->old_state);

	if(probe_data->second_element) {
		gst_element_set_state (probe_data->second_element, probe_data->old_state);
	}

	probe_data->done = true;

	return GST_PAD_PROBE_DROP;
}


void Changeable::add_element(GstElement* element, GstElement* first_element, GstElement* second_element)
{
	GstElement* pipeline = GST_ELEMENT(gst_element_get_parent(first_element));
	gchar* element_name = gst_element_get_name(element);

	sp_log(Log::Debug, this) << "Add " << element_name << " to pipeline";

	if(gst_bin_get_by_name((GstBin*)pipeline, element_name) != nullptr){
		sp_log(Log::Debug, this) << "Element already in pipeline";
		return;
	}

	GstPad* pad = gst_element_get_static_pad(first_element, "src");
	ProbeData* data = new ProbeData();
	data->first_element = first_element;
	data->second_element = second_element;
	data->element_of_interest = element;
	data->pipeline = pipeline;

	gst_element_get_state(pipeline, &data->old_state, nullptr, 0);

	if(data->old_state == GST_STATE_NULL)
	{
		if(data->second_element){
			gst_element_unlink(data->first_element, data->second_element);
		}

		gst_bin_add(GST_BIN(pipeline), data->element_of_interest);
		gst_element_link(data->first_element, data->element_of_interest);
		if(data->second_element){
			gst_element_link(data->element_of_interest, data->second_element);
		}

		sp_log(Log::Debug, this) << "Pipeline not playing, added " << element_name << " immediately";

		return;
	}

	gulong id = gst_pad_add_probe (pad,
								   GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
								   src_blocked_add,
								   data,
								   nullptr);

	Q_UNUSED(id)

	while(!data->done)
	{
		Util::sleep_ms(SleepInterval);
	}

	sp_log(Log::Debug, this) << "Element " << element_name << " added.";

	delete data; data = nullptr;
}


static GstPadProbeReturn
eos_probe_installed_remove(GstPad* pad, GstPadProbeInfo * info, gpointer data)
{
	ProbeData* probe_data = (ProbeData*) data;

	if(GST_EVENT_TYPE(GST_PAD_PROBE_INFO_DATA(info)) != GST_EVENT_EOS){
		return GST_PAD_PROBE_PASS;
	}

	gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

	gst_element_set_state(probe_data->first_element, GST_STATE_NULL);

	gst_element_unlink(probe_data->first_element, probe_data->element_of_interest);
	if(probe_data->second_element) {
		gst_element_unlink(probe_data->element_of_interest, probe_data->second_element);
	}

	gst_bin_remove(GST_BIN(probe_data->pipeline), probe_data->element_of_interest);
	gst_element_set_state(probe_data->element_of_interest, GST_STATE_NULL);

	if(probe_data->second_element)
	{
		gst_element_link(probe_data->first_element, probe_data->second_element);
		gst_element_set_state(probe_data->first_element, probe_data->old_state);
		gst_element_set_state(probe_data->second_element, probe_data->old_state);
	}

	probe_data->done = true;

	return GST_PAD_PROBE_DROP;
}


static GstPadProbeReturn
src_blocked_remove(GstPad* pad, GstPadProbeInfo* info, gpointer data)
{
	ProbeData* probe_data = static_cast<ProbeData*>(data);

	gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));

	GstPad* srcpad = gst_element_get_static_pad(probe_data->element_of_interest, "src");
	gst_pad_add_probe (srcpad,
					   (GstPadProbeType)(GST_PAD_PROBE_TYPE_BLOCK | GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM),
					   eos_probe_installed_remove,
					   probe_data,
					   NULL);

	gst_object_unref (srcpad);

	GstPad* sinkpad = gst_element_get_static_pad (probe_data->element_of_interest, "sink");
	gst_pad_send_event (sinkpad, gst_event_new_eos ());
	gst_object_unref (sinkpad);

	return GST_PAD_PROBE_OK;
}

#include <memory>
void Changeable::remove_element(GstElement* element, GstElement* first_element, GstElement* second_element)
{
	if(gst_element_get_parent(element) == nullptr){
		return;
	}

	GstElement* pipeline = GST_ELEMENT(gst_element_get_parent(first_element));
	char* element_name = gst_element_get_name(element);

	if(!gst_bin_get_by_name(GST_BIN(pipeline), element_name))
	{
		sp_log(Log::Debug, this) << "Element " << element_name << " not in pipeline";
		g_free(element_name);
		return;
	}

	GstPad* pad = gst_element_get_static_pad(first_element, "src");

	ProbeData* data = new ProbeData();
		data->first_element = first_element;
		data->second_element = second_element;
		data->element_of_interest = element;
		data->pipeline = pipeline;

	gst_element_get_state(pipeline, &data->old_state, nullptr, 0);

	if(data->old_state == GST_STATE_NULL)
	{
		gst_element_unlink(first_element, element);
		if(second_element){
			gst_element_unlink(element, second_element);
		}

		gst_bin_remove(GST_BIN(pipeline), element);

		if(second_element){
			gst_element_link(first_element, second_element);
		}

		sp_log(Log::Debug, this) << "Pipeline not playing, removed " << element_name << " immediately";
		g_free(element_name);
		return;
	}

	gulong id = gst_pad_add_probe (pad,
								   GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
								   src_blocked_remove,
								   data,
								   nullptr);
	Q_UNUSED(id)

	while(!data->done)
	{
		Util::sleep_ms(SleepInterval);
	}

	sp_log(Log::Debug, this) << "Element " << element_name << " removed.";

	delete data; data = nullptr;
	g_free(element_name);
}



