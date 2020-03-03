/* BroadcastBin.cpp */

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

#include "BroadcastBin.h"
#include "Probing.h"
#include "Components/Engine/EngineUtils.h"
#include "Components/Engine/Callbacks.h"

#include <QString>
#include <QList>

#include <gst/app/gstappsink.h>

using namespace PipelineExtensions;

struct BroadcastBin::Private
{
	BroadcastDataReceiver* dataReceiver=nullptr;
	GstElement* pipeline=nullptr;
	GstElement* tee=nullptr;

	GstElement*			bin=nullptr;
	GstElement*			queue=nullptr;
	GstElement*			converter=nullptr;
	GstElement*			resampler=nullptr;
	GstElement*			lame=nullptr;
	GstElement*			appSink=nullptr;

	gulong				probe;

	bool				isRunning;

	Private(BroadcastDataReceiver* dataReceiver, GstElement* pipeline, GstElement* tee) :
		dataReceiver(dataReceiver),
		pipeline(pipeline),
		tee(tee),
		probe(0),
		isRunning(false)
	{}
};

BroadcastBin::BroadcastBin(BroadcastDataReceiver* dataReceiver, GstElement* pipeline, GstElement* tee)
{
	m = Pimpl::make<Private>(dataReceiver, pipeline, tee);
}

BroadcastBin::~BroadcastBin() = default;

bool BroadcastBin::init()
{
	if(m->bin){
		return true;
	}

	// create
	if( !Engine::Utils::createElement(&m->queue, "queue", "bc_lame_queue") ||
		!Engine::Utils::createElement(&m->converter, "audioconvert", "bc_lame_converter") ||
		!Engine::Utils::createElement(&m->resampler, "audioresample", "bc_lame_resampler") ||
		!Engine::Utils::createElement(&m->lame, "lamemp3enc", "bc_lamemp3enc") ||
		!Engine::Utils::createElement(&m->appSink, "appsink", "bc_lame_appsink"))
	{
		return false;
	}

	{ // init bin
		bool success = Engine::Utils::createBin(&m->bin, {m->queue,  m->converter, m->resampler, m->lame, m->appSink}, "broadcast");
		if(!success){
			return false;
		}

		gst_bin_add(GST_BIN(m->pipeline), m->bin);
		success = Engine::Utils::connectTee(m->tee, m->bin, "BroadcastQueue");
		if(!success)
		{
			Engine::Utils::setState(m->bin, GST_STATE_NULL);
			gst_object_unref(m->bin);
			return false;
		}
	}

	{ // configure
		gst_object_ref(m->appSink);

		Engine::Utils::configureLame(m->lame);
		Engine::Utils::configureQueue(m->queue);
		Engine::Utils::configureSink(m->appSink);
		Engine::Utils::setValues(G_OBJECT(m->appSink), "emit-signals", true);

		g_signal_connect (m->appSink, "new-sample", G_CALLBACK(Engine::Callbacks::newBuffer), m->dataReceiver);
	}

	return true;
}

bool BroadcastBin::setEnabled(bool b)
{
	if(b && !init()){
		return false;
	}

	if(m->isRunning == b) {
		return true;
	}

	m->isRunning = b;
	Probing::handleProbe(&m->isRunning, m->queue, &m->probe, Probing::lameProbed);

	return true;
}

bool BroadcastBin::isEnabled() const
{
	return m->isRunning;
}
