/* Broadcaster.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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



#include "Broadcaster.h"
#include "Probing.h"
#include "Components/Engine/EngineUtils.h"
#include "Components/Engine/Callbacks.h"

#include <QString>
#include <QList>

using namespace PipelineExtensions;

struct Broadcaster::Private
{
	BroadcastDataReceiver* data_receiver=nullptr;
	GstElement* pipeline=nullptr;
	GstElement* tee=nullptr;

	GstElement*			bc_bin=nullptr;
	GstElement*			bc_queue=nullptr;
	GstElement*			bc_converter=nullptr;
	GstElement*			bc_resampler=nullptr;
	GstElement*			bc_lame=nullptr;
	GstElement*			bc_app_sink=nullptr;

	gulong				bc_probe;

	bool				is_running;

	Private(BroadcastDataReceiver* data_receiver, GstElement* pipeline, GstElement* tee) :
		data_receiver(data_receiver),
		pipeline(pipeline),
		tee(tee),
		bc_probe(0),
		is_running(false)
	{}
};

Broadcaster::Broadcaster(BroadcastDataReceiver* data_receiver, GstElement* pipeline, GstElement* tee)
{
	m = Pimpl::make<Private>(data_receiver, pipeline, tee);
}

Broadcaster::~Broadcaster() = default;

bool Broadcaster::init()
{
	if(m->bc_bin){
		return true;
	}

	// create
	if( !Engine::Utils::create_element(&m->bc_queue, "queue", "lame_queue") ||
		!Engine::Utils::create_element(&m->bc_converter, "audioconvert", "lame_converter") ||
		!Engine::Utils::create_element(&m->bc_resampler, "audioresample", "lame_resampler") ||
		!Engine::Utils::create_element(&m->bc_lame, "lamemp3enc") ||
		!Engine::Utils::create_element(&m->bc_app_sink, "appsink", "lame_appsink"))
	{
		return false;
	}

	{ // init bin
		bool success = Engine::Utils::create_bin(&m->bc_bin, {m->bc_queue,  m->bc_converter, m->bc_resampler, m->bc_lame, m->bc_app_sink}, "broadcast");
		if(!success){
			return false;
		}

		gst_bin_add(GST_BIN(m->pipeline), m->bc_bin);
		success = Engine::Utils::tee_connect(m->tee, m->bc_bin, "BroadcastQueue");
		if(!success)
		{
			Engine::Utils::set_state(m->bc_bin, GST_STATE_NULL);
			gst_object_unref(m->bc_bin);
			return false;
		}
	}

	{ // configure
		gst_object_ref(m->bc_app_sink);

		Engine::Utils::config_lame(m->bc_lame);
		Engine::Utils::config_queue(m->bc_queue);
		Engine::Utils::config_sink(m->bc_app_sink);
		Engine::Utils::set_values(G_OBJECT(m->bc_app_sink), "emit-signals", true);

		g_signal_connect (m->bc_app_sink, "new-sample", G_CALLBACK(Engine::Callbacks::new_buffer), m->data_receiver);
	}

	return true;
}

bool Broadcaster::set_enabled(bool b)
{
	if(b && !init()){
		return false;
	}

	m->is_running = b;
	Probing::handle_probe(&m->is_running, m->bc_queue, &m->bc_probe, Probing::lame_probed);

	return true;
}
