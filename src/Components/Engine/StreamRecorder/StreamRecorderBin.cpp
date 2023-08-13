/* StreamRecorderHandler.cpp */

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

#include "StreamRecorderData.h"
#include "StreamRecorderBin.h"

#include "Components/Engine/EngineUtils.h"
#include "Components/Engine/PipelineExtensions/Probing.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/StandardPaths.h"

#include <QString>

using StreamRecorder::Data;
using namespace PipelineExtensions;

namespace
{
	void configureBinElements(GstElement* lame, GstElement* queue, GstElement* sink)
	{
		Engine::Utils::configureLame(lame);
		Engine::Utils::configureQueue(queue);
		Engine::Utils::configureSink(sink);

		// this is just to avoid endless warning messages
		Engine::Utils::setValues(
			G_OBJECT(sink),
			"location", Util::tempPath(".stream-recorder.mp3").toLocal8Bit().data());

		Engine::Utils::setUintValue(G_OBJECT(sink), "buffer-size", 8192);
	}
}

struct StreamRecorderBin::Private
{
	GstElement* pipeline;
	GstElement* tee;

	GstElement* bin = nullptr;
	GstElement* queue = nullptr;
	GstElement* converter = nullptr;
	GstElement* sink = nullptr;
	GstElement* resampler = nullptr;
	GstElement* lame = nullptr;

	Data data;
	QString path;
	bool isRunning {false};

	Private(GstElement* pipeline, GstElement* tee) :
		pipeline(pipeline),
		tee(tee) {}
};

StreamRecorderBin::StreamRecorderBin(GstElement* pipeline, GstElement* tee) :
	m {Pimpl::make<Private>(pipeline, tee)} {}

StreamRecorderBin::~StreamRecorderBin() = default;

bool StreamRecorderBin::init()
{
	if(m->bin)
	{
		return true;
	}

	// stream recorder branch
	if(!Engine::Utils::createElement(&m->queue, "queue", "sr_queue") ||
	   !Engine::Utils::createElement(&m->converter, "audioconvert", "sr_converter") ||
	   !Engine::Utils::createElement(&m->resampler, "audioresample", "sr_resample") ||
	   !Engine::Utils::createElement(&m->lame, "lamemp3enc", "sr_lame") ||
	   !Engine::Utils::createElement(&m->sink, "filesink", "sr_filesink"))
	{
		return false;
	}

	m->data.queue = m->queue;
	m->data.sink = m->sink;

	configureBinElements(m->lame, m->queue, m->sink);

	const auto success = Engine::Utils::createBin(
		&m->bin,
		{m->queue, m->converter, m->resampler, m->lame, m->sink},
		"sr");

	if(!success)
	{
		return false;
	}

	gst_bin_add(GST_BIN(m->pipeline), m->bin);

	const auto teeConnected = Engine::Utils::connectTee(m->tee, m->bin, "StreamRecorderQueue");
	if(!teeConnected)
	{
		Engine::Utils::setState(m->bin, GST_STATE_NULL);
		gst_object_unref(m->bin);
	}

	return teeConnected;

}

bool StreamRecorderBin::setEnabled(const bool b)
{
	return b ? init() : true;
}

void StreamRecorderBin::setTargetPath(const QString& path)
{
	if(!m->sink)
	{
		return;
	}

	if(const auto isNewPath = (path != m->path && !path.isEmpty()); !isNewPath || m->data.busy)
	{
		return;
	}

	m->path = path;
	m->isRunning = !(path.isEmpty());

	auto* oldFilename = m->data.filename;

	m->data.filename = strdup(m->path.toUtf8().data());
	m->data.active = m->isRunning;

	Probing::handleStreamRecorderProbe(&m->data, Probing::streamRecorderProbed);

	if(oldFilename)
	{
		free(oldFilename);
	}
}
