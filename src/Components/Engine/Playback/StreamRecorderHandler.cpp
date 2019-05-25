/* StreamRecorderHandler.cpp */

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

#include "StreamRecorderHandler.h"
#include "PipelineProbes.h"
#include "StreamRecorderData.h"

#include "Utils/Settings/Settings.h"
#include <QString>

using Pipeline::StreamRecorderHandler;

struct StreamRecorderHandler::Private
{
	QString				sr_path;
	bool				run_sr;
	StreamRecorder::Data* sr_data=nullptr;

	Private() :
		sr_data(new StreamRecorder::Data())
	{}

	~Private()
	{
		delete sr_data; sr_data = nullptr;
	}
};

StreamRecorderHandler::StreamRecorderHandler()
{
	m = Pimpl::make<Private>();
}

StreamRecorderHandler::~StreamRecorderHandler() {}

StreamRecorder::Data* StreamRecorderHandler::streamrecorder_data() const
{
	return m->sr_data;
}

void StreamRecorderHandler::set_streamrecorder_target_path(const QString& path)
{
	GstElement* file_sink_element = get_streamrecorder_sink_element();
	if(!file_sink_element) {
		return;
	}

	if(path == m->sr_path && !m->sr_path.isEmpty()) {
		return;
	}

	if(m->sr_data->busy){
		return;
	}

	m->sr_path = path;
	m->run_sr = !(path.isEmpty());

	gchar* old_filename = m->sr_data->filename;

	m->sr_data->filename = strdup(m->sr_path.toUtf8().data());
	m->sr_data->active = m->run_sr;

	Probing::handle_stream_recorder_probe(m->sr_data, Probing::stream_recorder_probed);

	if(old_filename){
		free(old_filename);
	}
}

