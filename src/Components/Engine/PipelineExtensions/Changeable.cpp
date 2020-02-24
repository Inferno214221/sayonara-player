/* ChangeablePipeline.cpp */

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

#include "Changeable.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Components/Engine/EngineUtils.h"

#include <QList>

#include <memory>
#include <mutex>

static std::mutex mtx;

using PipelineExtensions::Changeable;

const int SleepInterval = 50;

struct StackMutex
{
	bool couldLock;
	StackMutex(std::mutex& mtx) :
		couldLock(mtx.try_lock())
	{}

	~StackMutex()
	{
		if(couldLock)
		{
			mtx.unlock();
		}
	}
};

struct ProbeData
{
	GstElement* firstElement=nullptr;
	GstElement* secondElement=nullptr;
	GstElement* elementOfInterest=nullptr;
	GstElement* pipeline=nullptr;

	GstState oldState;
	bool done;

	ProbeData()
	{
		oldState = GST_STATE_NULL;
		done = false;
	}
};

Changeable::Changeable() {}
Changeable::~Changeable() {}


static GstPadProbeReturn
src_blocked_add(GstPad* pad, GstPadProbeInfo* info, gpointer data)
{
	auto* probeData = static_cast<ProbeData*>(data);

	gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));

	Engine::Utils::setState(probeData->firstElement, GST_STATE_NULL);
	Engine::Utils::setState(probeData->elementOfInterest, GST_STATE_NULL);

	Engine::Utils::addElements(GST_BIN(probeData->pipeline), {probeData->elementOfInterest});
	Engine::Utils::unlinkElements({probeData->firstElement, probeData->secondElement});

	Engine::Utils::linkElements({
		probeData->firstElement, probeData->elementOfInterest, probeData->secondElement
	});

	Engine::Utils::setState(probeData->elementOfInterest, probeData->oldState);
	Engine::Utils::setState(probeData->firstElement, probeData->oldState);

	if(probeData->secondElement)
	{
		Engine::Utils::setState(probeData->secondElement, probeData->oldState);
	}

	probeData->done = true;

	return GST_PAD_PROBE_DROP;
}


bool Changeable::addElement(GstElement* element, GstElement* firstElement, GstElement* secondElement)
{
	StackMutex sm(mtx);
	if(!sm.couldLock) {
		return false;
	}

	Engine::Utils::GStringAutoFree name(gst_element_get_name(element));

	spLog(Log::Debug, this) << "Add " << name.data() << " to pipeline";

	GstElement* parent = GST_ELEMENT(gst_element_get_parent(firstElement));
	if(Engine::Utils::hasElement(GST_BIN(parent), element))
	{
		spLog(Log::Debug, this) << "Element already in pipeline";
		return true;
	}

	auto data = std::make_shared<ProbeData>();
		data->firstElement = firstElement;
		data->secondElement = secondElement;
		data->elementOfInterest = element;
		data->pipeline = parent;

	data->oldState = Engine::Utils::getState(parent);
	if(data->oldState != GST_STATE_PLAYING)
	{
		Engine::Utils::unlinkElements({data->firstElement, data->secondElement});

		bool success = Engine::Utils::addElements(GST_BIN(parent), {data->elementOfInterest});
		if(success)
		{
			success = Engine::Utils::linkElements({
				data->firstElement, data->elementOfInterest, data->secondElement
			});

			if(!success)
			{
				spLog(Log::Warning, this) << "Could not link elements for any reason";
				Engine::Utils::removeElements(GST_BIN(parent), {data->elementOfInterest});
			}
		}

		if(success){
			spLog(Log::Debug, this) << "Pipeline not playing, added " << name.data() << " immediately";
		}

		return success;
	}

	GstPad* pad = gst_element_get_static_pad(firstElement, "src");
	auto probe_id = gst_pad_add_probe
	(
		pad,
		GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
		src_blocked_add,
		data.get(), nullptr
	);

	int32_t MaxMs=2000;
	bool success = true;
	while(!data->done)
	{
		Util::sleepMs(SleepInterval);
		MaxMs -= SleepInterval;
		if(MaxMs <= 0)
		{
			success = false;
			spLog(Log::Warning, this) << "Add: Could not establish probe " << name.data();
			gst_pad_remove_probe(pad, probe_id);
			break;
		}
	}

	if(success)
	{
		spLog(Log::Debug, this) << "Element " << name.data() << " added.";
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

	Engine::Utils::setState(probe_data->firstElement, GST_STATE_NULL);
	Engine::Utils::unlinkElements({
		probe_data->firstElement, probe_data->elementOfInterest, probe_data->secondElement
	});

	Engine::Utils::removeElements(GST_BIN(probe_data->pipeline), {probe_data->elementOfInterest});
	Engine::Utils::setState(probe_data->elementOfInterest, GST_STATE_NULL);

	if(probe_data->secondElement)
	{
		Engine::Utils::linkElements({probe_data->firstElement, probe_data->secondElement});
		Engine::Utils::setState(probe_data->firstElement, probe_data->oldState);
		Engine::Utils::setState(probe_data->secondElement, probe_data->oldState);
	}

	probe_data->done = true;

	return GST_PAD_PROBE_DROP;
}


static GstPadProbeReturn
src_blocked_remove(GstPad* pad, GstPadProbeInfo* info, gpointer data)
{
	auto* probe_data = static_cast<ProbeData*>(data);

	gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

	/** We only need that second probe here to flush all next elements **/
	if(probe_data->secondElement)
	{
		GstPad* srcpad = gst_element_get_static_pad(probe_data->elementOfInterest, "src");
		gst_pad_add_probe
		(
			srcpad,
			GstPadProbeType(GST_PAD_PROBE_TYPE_BLOCK | GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM),
			eos_probe_installed_remove,
			probe_data, nullptr
		);

		gst_object_unref (srcpad);
	}

	/** Flush the old element with an eos event **/
	GstPad* sinkpad = gst_element_get_static_pad (probe_data->elementOfInterest, "sink");
	gst_pad_send_event (sinkpad, gst_event_new_eos());
	gst_object_unref (sinkpad);

	if(!probe_data->secondElement)
	{
		Engine::Utils::removeElements(GST_BIN(probe_data->pipeline), {probe_data->elementOfInterest});
		probe_data->done = true;
	}

	return GST_PAD_PROBE_OK;
}


bool Changeable::removeElement(GstElement* element, GstElement* firstElement, GstElement* secondElement)
{
	StackMutex sm(mtx);
	if(!sm.couldLock) {
		return false;
	}

	Engine::Utils::GStringAutoFree name(gst_element_get_name(element));

	spLog(Log::Debug, this) << "Remove " << name.data() << " from pipeline";

	GstElement* parent = GST_ELEMENT(gst_element_get_parent(firstElement));
	if(!Engine::Utils::hasElement(GST_BIN(parent), element))
	{
		spLog(Log::Debug, this) << "Element " << name.data() << " not in pipeline";
		return true;
	}

	GstPad* pad = gst_element_get_static_pad(firstElement, "src");

	auto data = std::make_shared<ProbeData>();
		data->firstElement = firstElement;
		data->secondElement = secondElement;
		data->elementOfInterest = element;
		data->pipeline = parent;

	data->oldState = Engine::Utils::getState(parent);

	// we need that element later, but a gst_bin_remove decreases refcount
	gst_object_ref(element);

	if(data->oldState != GST_STATE_PLAYING)
	{
		Engine::Utils::unlinkElements({firstElement, element, secondElement});
		Engine::Utils::removeElements(GST_BIN(parent), {element});

		bool success = true;
		if(secondElement) {
			success = Engine::Utils::linkElements({firstElement, secondElement});
		}

		if(success) {
			spLog(Log::Debug, this) << "Pipeline not playing, removed " << name.data() << " immediately";
		}

		return success;
	}

	gst_pad_add_probe
	(
		pad,
		GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
		src_blocked_remove,
		data.get(), nullptr
	);

	int32_t MaxMs=2000;
	bool success = true;
	while(!data->done)
	{
		Util::sleepMs(SleepInterval);
		MaxMs -= SleepInterval;
		if(MaxMs <= 0)
		{
			success = false;
			spLog(Log::Warning, this) << "Remove: Could not establish probe callback " << name.data();
			break;
		}
	}

	if(success)
	{
		spLog(Log::Debug, this) << "Element " << name.data() << " removed.";
	}

	return success;
}

struct ReplaceSinkProbeData
{
	GstElement* old_sink=nullptr;
	GstElement* new_sink=nullptr;
	GstElement* element_before=nullptr;
	GstElement* pipeline=nullptr;
	GstElement*	bin=nullptr;
	bool done;
	GstState old_state;

	ReplaceSinkProbeData() : done(false) {}
};


static GstPadProbeReturn
src_blocked_replace(GstPad* pad, GstPadProbeInfo* info, gpointer data)
{
	auto* probe_data = static_cast<ReplaceSinkProbeData*>(data);

	gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

	Engine::Utils::setState(probe_data->old_sink, GST_STATE_NULL);
	Engine::Utils::removeElements(GST_BIN(probe_data->bin), {probe_data->old_sink});

	Engine::Utils::addElements(GST_BIN(probe_data->bin), {probe_data->new_sink});
	Engine::Utils::linkElements({probe_data->element_before, probe_data->new_sink});

	gst_element_sync_state_with_parent(probe_data->new_sink);

	probe_data->done = true;

	spLog(Log::Debug, "Changeable ReplaceCB") << "Replacement finished";

	return GST_PAD_PROBE_OK;
}


bool Changeable::replaceSink(GstElement* old_sink, GstElement* new_sink, GstElement* element_before, GstElement* pipeline, GstElement* bin)
{
	StackMutex sm(mtx);
	if(!sm.couldLock) {
		return false;
	}

	Engine::Utils::GStringAutoFree name( gst_element_get_name(old_sink)	);
	spLog(Log::Debug, this) << "Remove " << name.data() << " from pipeline";

	//gst_element* bin = gst_element(gst_element_get_parent(old_sink));
	GstPad* pad = gst_element_get_static_pad(element_before, "src");
	GstState old_state = Engine::Utils::getState(old_sink);

	if(!Engine::Utils::hasElement(GST_BIN(bin), old_sink))
	{
		spLog(Log::Debug, this) << "Element " << name.data() << " not in pipeline";
		return addElement(new_sink, element_before, nullptr);
	}

	auto* probe_data = new ReplaceSinkProbeData();
		probe_data->old_sink = old_sink;
		probe_data->new_sink = new_sink;
		probe_data->element_before = element_before;
		probe_data->pipeline = pipeline;
		probe_data->bin = bin;
		probe_data->old_state = old_state;

	//gst_object_ref(old_sink);

	if(probe_data->old_state == GST_STATE_NULL)
	{
		//Engine::Utils::set_state(old_sink, GST_STATE_NULL);
		Engine::Utils::removeElements(GST_BIN(bin), {old_sink});
		Engine::Utils::addElements(GST_BIN(bin), {new_sink});
		Engine::Utils::linkElements({element_before, new_sink});

		spLog(Log::Debug, this) << "Immediate replacement finished";

		return true;
	}

	gst_pad_add_probe
	(
		pad,
		GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
		src_blocked_replace,
		probe_data, nullptr
	);

	spLog(Log::Debug, this) << "Element " << name.data() << " replaced.";

	MilliSeconds maxMs = 2000;
	while(Engine::Utils::getState(pipeline) != old_state)
	{
		Util::sleepMs(50);
		maxMs -= 50;
		if(maxMs < 0){
			return false;
		}
	}

	return true;
}
