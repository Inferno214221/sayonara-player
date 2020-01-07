/* Visualizer.cpp */

/* Copyright (C) 2011-2020 Lucio Carreras
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



#include "Visualizer.h"
#include "Probing.h"
#include "Components/Engine/EngineUtils.h"

#include "Utils/Settings/Settings.h"

#include <QString>
#include <QList>

using namespace PipelineExtensions;

struct Visualizer::Private
{
	GstElement* pipeline=nullptr;
	GstElement* tee=nullptr;

	GstElement*			visualizer_bin=nullptr;
	GstElement*			visualizer_queue=nullptr;
	GstElement*			spectrum=nullptr;
	GstElement*			level=nullptr;
	GstElement*			visualizer_sink=nullptr;

	gulong				probe;
	bool				is_running;

	Private(GstElement* pipeline, GstElement* tee) :
		pipeline(pipeline),
		tee(tee),
		probe(0),
		is_running(false)
	{}
};

Visualizer::Visualizer(GstElement* pipeline, GstElement* tee)
{
	m = Pimpl::make<Private>(pipeline, tee);
}

Visualizer::~Visualizer() = default;

bool Visualizer::init()
{
	if(m->visualizer_bin){
		return true;
	}

	{ // create
		if(	Engine::Utils::create_element(&m->visualizer_queue, "queue", "visualizer") &&
			Engine::Utils::create_element(&m->level, "level") &&	// in case of renaming, also look in EngineCallbase GST_MESSAGE_EVENT
			Engine::Utils::create_element(&m->spectrum, "spectrum") &&
			Engine::Utils::create_element(&m->visualizer_sink,"fakesink", "visualizer"))
		{
			Engine::Utils::create_bin(&m->visualizer_bin, {m->visualizer_queue, m->level, m->spectrum, m->visualizer_sink}, "visualizer");
		}

		if(!m->visualizer_bin){
			return false;
		}
	}

	{ // link
		gst_bin_add(GST_BIN(m->pipeline), m->visualizer_bin);
		bool success = Engine::Utils::tee_connect(m->tee, m->visualizer_bin, "Visualizer");
		if(!success)
		{
			gst_bin_remove(GST_BIN(m->pipeline), m->visualizer_bin);
			gst_object_unref(m->visualizer_bin);
			m->visualizer_bin = nullptr;
			return false;
		}
	}

	{ // configure
		Engine::Utils::set_values(G_OBJECT(m->level), "post-messages", true);
		Engine::Utils::set_uint64_value(G_OBJECT(m->level), "interval", 20 * GST_MSECOND);
		Engine::Utils::set_values(G_OBJECT (m->spectrum),
					  "post-messages", true,
					  "message-phase", false,
					  "message-magnitude", true,
					  "multi-channel", false);

		Engine::Utils::set_int_value(G_OBJECT(m->spectrum), "threshold", -75);
		Engine::Utils::set_uint_value(G_OBJECT(m->spectrum), "bands", GetSetting(Set::Engine_SpectrumBins));
		Engine::Utils::set_uint64_value(G_OBJECT(m->spectrum), "interval", 20 * GST_MSECOND);

		Engine::Utils::config_queue(m->visualizer_queue, 1000);
		Engine::Utils::config_sink(m->visualizer_sink);
	}

	return true;
}

bool Visualizer::set_enabled(bool b)
{
	if(!init()){
		return false;
	}

	m->is_running = b;
	Probing::handle_probe(&m->is_running, m->visualizer_queue, &m->probe, Probing::spectrum_probed);

	bool show_level = GetSetting(Set::Engine_ShowLevel);
	bool show_spectrum = GetSetting(Set::Engine_ShowSpectrum);

	Engine::Utils::set_value(G_OBJECT(m->level), "post-messages", show_level);
	Engine::Utils::set_value(G_OBJECT(m->spectrum), "post-messages", show_spectrum);

	return true;
}
