/* StreamRecorderHandler.cpp */

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

#include "StreamRecorderData.h"
#include "StreamRecorderHandler.h"

#include "Components/Engine/EngineUtils.h"
#include "Components/Engine/PipelineExtensions/Probing.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include <QString>

using StreamRecorder::Data;
using namespace PipelineExtensions;

struct StreamRecorderHandler::Private
{
	GstElement*			pipeline=nullptr;
	GstElement*			tee=nullptr;

	GstElement*			sr_bin=nullptr;
	GstElement*			sr_queue=nullptr;
	GstElement*			sr_converter=nullptr;
	GstElement*			sr_sink=nullptr;
	GstElement*			sr_resampler=nullptr;
	GstElement*			sr_lame=nullptr;

	Data*				data=nullptr;
	QString				path;
	bool				is_running;


	Private(GstElement* pipeline, GstElement* tee) :
		pipeline(pipeline),
		tee(tee),
		data(new Data()),
		is_running(false)
	{}

	~Private()
	{
		delete data; data = nullptr;
	}
};

StreamRecorderHandler::StreamRecorderHandler(GstElement* pipeline, GstElement* tee)
{
	m = Pimpl::make<Private>(pipeline, tee);
}

StreamRecorderHandler::~StreamRecorderHandler() {}

bool StreamRecorderHandler::init()
{
	if(m->sr_bin) {
		return true;
	}

	// stream recorder branch
	if(	!Engine::Utils::create_element(&m->sr_queue, "queue", "sr_queue") ||
		!Engine::Utils::create_element(&m->sr_converter, "audioconvert", "sr_converter") ||
		!Engine::Utils::create_element(&m->sr_resampler, "audioresample", "sr_resample") ||
		!Engine::Utils::create_element(&m->sr_lame, "lamemp3enc", "sr_lame")  ||
		!Engine::Utils::create_element(&m->sr_sink, "filesink", "sr_filesink"))
	{
		return false;
	}

	m->data->queue = m->sr_queue;
	m->data->sink = m->sr_sink;

	{ // configure
		Engine::Utils::config_lame(m->sr_lame);
		Engine::Utils::config_queue(m->sr_queue);
		Engine::Utils::config_sink(m->sr_sink);

		Engine::Utils::set_values(G_OBJECT(m->sr_sink),
								"location", Util::sayonara_path("bla.mp3").toLocal8Bit().data());
		Engine::Utils::set_uint_value(G_OBJECT(m->sr_sink), "buffer-size", 8192);
	}

	{ // init bin
		bool success = Engine::Utils::create_bin(&m->sr_bin, {m->sr_queue, m->sr_converter, m->sr_resampler, m->sr_lame, m->sr_sink}, "sr");
		if(!success){
			return false;
		}

		gst_bin_add(GST_BIN(m->pipeline), m->sr_bin);

		success = Engine::Utils::tee_connect(m->tee, m->sr_bin, "StreamRecorderQueue");
		if(!success)
		{
			Engine::Utils::set_state(m->sr_bin, GST_STATE_NULL);
			gst_object_unref(m->sr_bin);
		}

		return success;
	}
}

bool StreamRecorderHandler::set_enabled(bool b)
{
	if(b){
		return init();
	}

	return true;
}


void StreamRecorderHandler::set_target_path(const QString& path)
{
	GstElement* file_sink_element = m->sr_sink;
	if(!file_sink_element) {
		return;
	}

	if(path == m->path && !m->path.isEmpty()) {
		return;
	}

	if(m->data->busy){
		return;
	}

	m->path = path;
	m->is_running = !(path.isEmpty());

	gchar* old_filename = m->data->filename;

	m->data->filename = strdup(m->path.toUtf8().data());
	m->data->active = m->is_running;

	Probing::handle_stream_recorder_probe(m->data, Probing::stream_recorder_probed);

	if(old_filename){
		free(old_filename);
	}
}

