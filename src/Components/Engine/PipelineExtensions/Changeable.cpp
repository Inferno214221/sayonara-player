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

#include "Changeable.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Components/Engine/Utils.h"

#include <QList>

#include <memory>
#include <mutex>

static std::mutex mtx;

using PipelineExtensions::Changeable;

const int SleepInterval = 50;

struct StackMutex
{
	bool could_lock;
	StackMutex(std::mutex& mtx) :
		could_lock(mtx.try_lock())
	{}

	~StackMutex()
	{
		if(could_lock)
		{
			mtx.unlock();
		}
	}
};

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

Changeable::Changeable() = default;
Changeable::~Changeable() = default;


static GstPadProbeReturn
src_blocked_add(GstPad* pad, GstPadProbeInfo* info, gpointer data)
{
	ProbeData* probe_data = static_cast<ProbeData*>(data);

	gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));

	Engine::Utils::set_state(probe_data->first_element, GST_STATE_NULL);
	Engine::Utils::set_state(probe_data->element_of_interest, GST_STATE_NULL);

	Engine::Utils::add_elements(GST_BIN(probe_data->pipeline), {probe_data->element_of_interest});
	Engine::Utils::unlink_elements({probe_data->first_element, probe_data->second_element});

	Engine::Utils::link_elements({
		probe_data->first_element, probe_data->element_of_interest, probe_data->second_element
	});

	Engine::Utils::set_state(probe_data->element_of_interest, probe_data->old_state);
	Engine::Utils::set_state(probe_data->first_element, probe_data->old_state);

	if(probe_data->second_element)
	{
		Engine::Utils::set_state(probe_data->second_element, probe_data->old_state);
	}

	probe_data->done = true;

	return GST_PAD_PROBE_DROP;
}


bool Changeable::add_element(GstElement* element, GstElement* first_element, GstElement* second_element)
{
	StackMutex sm(mtx);
	if(!sm.could_lock) {
		return false;
	}

	Engine::Utils::GObjectAutoFree name
	(
		gst_element_get_name(element)
	);

	sp_log(Log::Debug, this) << "Add " << name.data() << " to pipeline";

	GstElement* pipeline = this->pipeline();
	if(Engine::Utils::has_element(GST_BIN(pipeline), element))
	{
		sp_log(Log::Debug, this) << "Element already in pipeline";
		return true;
	}

	auto data = std::make_shared<ProbeData>();
		data->first_element = first_element;
		data->second_element = second_element;
		data->element_of_interest = element;
		data->pipeline = pipeline;

	data->old_state = Engine::Utils::get_state(pipeline);
	if(data->old_state != GST_STATE_PLAYING)
	{
		Engine::Utils::unlink_elements({data->first_element, data->second_element});

		bool success = Engine::Utils::add_elements(GST_BIN(pipeline), {data->element_of_interest});
		if(success)
		{
			success = Engine::Utils::link_elements({
				data->first_element, data->element_of_interest, data->second_element
			});

			if(!success)
			{
				sp_log(Log::Warning, this) << "Could not link elements for any reason";
				Engine::Utils::remove_elements(GST_BIN(pipeline), {data->element_of_interest});
			}
		}

		if(success){
			sp_log(Log::Debug, this) << "Pipeline not playing, added " << name.data() << " immediately";
		}

		return success;
	}

	GstPad* pad = gst_element_get_static_pad(first_element, "src");
	auto probe_id = gst_pad_add_probe
	(
		pad,
		GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
		src_blocked_add,
		data.get(), nullptr
	);

	uint32_t MaxMs=1000;
	bool success = true;
	while(!data->done)
	{
		Util::sleep_ms(SleepInterval);
		MaxMs -= SleepInterval;
		if(MaxMs <= 0)
		{
			success = false;
			sp_log(Log::Warning, this) << "Add: Could not establish probe " << name.data();
			gst_pad_remove_probe(pad, probe_id);
			break;
		}
	}

	if(success)
	{
		sp_log(Log::Debug, this) << "Element " << name.data() << " added.";
	}

	return success;
}


static GstPadProbeReturn
eos_probe_installed_remove(GstPad* pad, GstPadProbeInfo * info, gpointer data)
{
	auto* probe_data = static_cast<ProbeData*>(data);

	if(GST_EVENT_TYPE(GST_PAD_PROBE_INFO_DATA(info)) != GST_EVENT_EOS){
		return GST_PAD_PROBE_PASS;
	}

	gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

	Engine::Utils::set_state(probe_data->first_element, GST_STATE_NULL);
	Engine::Utils::unlink_elements({
		probe_data->first_element, probe_data->element_of_interest, probe_data->second_element
	});

	Engine::Utils::remove_elements(GST_BIN(probe_data->pipeline), {probe_data->element_of_interest});
	Engine::Utils::set_state(probe_data->element_of_interest, GST_STATE_NULL);

	if(probe_data->second_element)
	{
		Engine::Utils::link_elements({probe_data->first_element, probe_data->second_element});
		Engine::Utils::set_state(probe_data->first_element, probe_data->old_state);
		Engine::Utils::set_state(probe_data->second_element, probe_data->old_state);
	}

	probe_data->done = true;

	return GST_PAD_PROBE_DROP;
}


static GstPadProbeReturn
src_blocked_remove(GstPad* pad, GstPadProbeInfo* info, gpointer data)
{
	auto* probe_data = static_cast<ProbeData*>(data);

	gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

	GstPad* srcpad = gst_element_get_static_pad(probe_data->element_of_interest, "src");
	gst_pad_add_probe
	(
		srcpad,
		GstPadProbeType(GST_PAD_PROBE_TYPE_BLOCK | GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM),
		eos_probe_installed_remove,
		probe_data, nullptr
	);

	gst_object_unref (srcpad);

	GstPad* sinkpad = gst_element_get_static_pad (probe_data->element_of_interest, "sink");
	gst_pad_send_event (sinkpad, gst_event_new_eos ());
	gst_object_unref (sinkpad);

	return GST_PAD_PROBE_OK;
}


bool Changeable::remove_element(GstElement* element, GstElement* first_element, GstElement* second_element)
{
	StackMutex sm(mtx);
	if(!sm.could_lock) {
		return false;
	}

	Engine::Utils::GObjectAutoFree name
	(
		gst_element_get_name(element)
	);

	sp_log(Log::Debug, this) << "Remove " << name.data() << " from pipeline";

	GstElement* pipeline = this->pipeline();
	if(!Engine::Utils::has_element(GST_BIN(pipeline), element))
	{
		sp_log(Log::Debug, this) << "Element " << name.data() << " not in pipeline";
		return true;
	}

	GstPad* pad = gst_element_get_static_pad(first_element, "src");

	auto data = std::make_shared<ProbeData>();
		data->first_element = first_element;
		data->second_element = second_element;
		data->element_of_interest = element;
		data->pipeline = pipeline;

	data->old_state = Engine::Utils::get_state(pipeline);

	// we need that element later, but a gst_bin_remove decreases refcount
	gst_object_ref(element);

	if(data->old_state != GST_STATE_PLAYING)
	{
		Engine::Utils::unlink_elements({first_element, element, second_element});
		Engine::Utils::remove_elements(GST_BIN(pipeline), {element});

		bool success = true;
		if(second_element) {
			success = Engine::Utils::link_elements({first_element, second_element});
		}

		if(success) {
			sp_log(Log::Debug, this) << "Pipeline not playing, removed " << name.data() << " immediately";
		}

		return success;
	}

	auto probe_id = gst_pad_add_probe
	(
		pad,
		GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
		src_blocked_remove,
		data.get(), nullptr
	);

	uint32_t MaxMs=1000;
	bool success = true;
	while(!data->done)
	{
		Util::sleep_ms(SleepInterval);
		MaxMs -= SleepInterval;
		if(MaxMs <= 0)
		{
			success = false;
			sp_log(Log::Warning, this) << "Remove: Could not establish probe callback " << name.data();
			gst_pad_remove_probe(pad, probe_id);
			break;
		}
	}

	if(success)
	{
		sp_log(Log::Debug, this) << "Element " << name.data() << " removed.";
	}

	return success;
}



