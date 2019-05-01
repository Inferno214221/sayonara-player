/* GSTPipeline.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "AbstractPipeline.h"
#include "Components/Engine/AbstractEngine.h"
#include "Callbacks/EngineUtils.h"
#include "Callbacks/EngineCallbacks.h"
#include "Callbacks/PipelineCallbacks.h"
#include "Utils/Logger/Logger.h"

#include <QTimer>
#include <gst/app/gstappsink.h>

using Pipeline::Base;
namespace EngineUtils=Engine::Utils;

struct Base::Private
{
	MilliSeconds	duration_ms;
	MilliSeconds	position_source_ms;
	MilliSeconds	position_pipeline_ms;

	QString			name;
	GstBus*			bus=nullptr;
	Engine::Base*   engine=nullptr;
	QTimer*         progress_timer=nullptr;
	gchar*			uri=nullptr;
	GstElement*		pipeline=nullptr;

	QList<GstElement*> elements;

	bool			about_to_finish;
	bool            initialized;

	Private(const QString& name, Engine::Base* engine) :
		duration_ms(0),
		position_source_ms(0),
		position_pipeline_ms(0),
		name(name),
		engine(engine),
		about_to_finish(false),
		initialized(false)
	{}
};

Base::Base(QString name, Engine::Base* engine, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(name, engine);
}

Base::~Base()
{
	if (m->pipeline)
	{
		gst_element_set_state(GST_ELEMENT(m->pipeline), GST_STATE_NULL);
		gst_object_unref (GST_OBJECT(m->pipeline));
		m->pipeline = nullptr;
	}
}


bool Base::init(GstState state)
{
	bool success = false;
	if(m->initialized) {
		return true;
	}

	// create equalizer element
	m->pipeline = gst_pipeline_new(m->name.toStdString().c_str());
	if(!EngineUtils::test_and_error(m->pipeline, "Engine: Pipeline sucks")){
		return false;
	}

	m->bus = gst_pipeline_get_bus(GST_PIPELINE(m->pipeline));

	success = create_elements();
	if(!success) {
		return false;
	}

	success = add_and_link_elements();
	if(!success) {
		return false;
	}

	configure_elements();

	EngineUtils::set_state(m->pipeline, state);

#ifdef Q_OS_WIN
	gst_bus_set_sync_handler(m->bus, Engine::Callbacks::bus_message_received, m->engine, EngineCallbacks::destroy_notify);
#else
	gst_bus_add_watch(m->bus, Engine::Callbacks::bus_state_changed, m->engine);
#endif

	m->progress_timer = new QTimer(this);
	m->progress_timer->setInterval(200);
	connect(m->progress_timer, &QTimer::timeout, this, [=]()
	{
		if(this->get_state() != GST_STATE_NULL){
			Callbacks::position_changed(this);
		}
	});

	m->progress_timer->start();

	m->initialized = true;
	return true;
}

void Base::refresh_position()
{
	GstElement* element = get_source();
	if(!element){
		element = GST_ELEMENT(m->pipeline);
	}

	MilliSeconds pos_source_ms = EngineUtils::get_position_ms(element);
	MilliSeconds pos_pipeline_ms = EngineUtils::get_position_ms(m->pipeline);

	m->position_source_ms = 0;
	m->position_pipeline_ms = 0;

	if(pos_source_ms >= 0) {
		m->position_source_ms = pos_source_ms;
	}

	if(pos_pipeline_ms >= 0) {
		m->position_pipeline_ms = pos_pipeline_ms;
	}

	emit sig_pos_changed_ms( m->position_pipeline_ms );
}

void Base::update_duration_ms(MilliSeconds duration_ms, GstElement *src)
{
	if(src == get_source()){
		m->duration_ms = duration_ms;
	}

	refresh_position();
}

void Base::set_data(Byte* data, uint64_t size)
{
	emit sig_data(data, size);
}


//static void show_time_info(TimestampMSecs pos, TimestampMSecs dur){
//	sp_log(Log::Develop, this) << "Difference: "
//					   << dur - pos << ": "
//					   << pos << " - "
//					   << dur;
//}


void Base::check_about_to_finish()
{
	if(!m->about_to_finish)
	{
		if(m->duration_ms < m->position_pipeline_ms || (m->duration_ms == 0))
		{
			m->duration_ms = EngineUtils::get_duration_ms(get_source());

			if(m->duration_ms < 0){
				return;
			}
		}
	}

	MilliSeconds about_to_finish_time = get_about_to_finish_time();
	if(!m->about_to_finish)
	{
		if(m->position_pipeline_ms <= m->duration_ms)
		{
			if((m->duration_ms - m->position_pipeline_ms) < about_to_finish_time)
			{
				m->about_to_finish = true;

				sp_log(Log::Develop, this) << "Duration: " << m->duration_ms << ", Position: " << m->position_pipeline_ms;

				emit sig_about_to_finish(m->duration_ms - m->position_pipeline_ms);
				return;
			}
		}
	}

	m->about_to_finish = false;
}

MilliSeconds Base::get_time_to_go() const
{
	GstElement* element = GST_ELEMENT(m->pipeline);
	MilliSeconds ms = EngineUtils::get_time_to_go(element);
	if(ms < 100){
		return 0;
	}

	return ms - 100;
}


MilliSeconds Base::get_duration_ms() const
{
	return m->duration_ms;
}

MilliSeconds Base::get_source_position_ms() const
{
	return m->position_source_ms;
}

MilliSeconds Base::get_pipeline_position_ms() const
{
	return m->position_pipeline_ms;
}

bool Base::has_element(GstElement* e) const
{
	return EngineUtils::has_element(GST_BIN(m->pipeline), e);
}

void Base::finished()
{
	emit sig_finished();
}


GstState Base::get_state()
{
	return EngineUtils::get_state(m->pipeline);
}


GstElement* Base::pipeline() const
{
	return m->pipeline;
}

bool Base::set_uri(gchar* uri)
{
	m->uri = uri;
	return (m->uri != nullptr);
}

MilliSeconds Base::get_about_to_finish_time() const
{
	return 300;
}

void Base::set_about_to_finish(bool b)
{
	m->about_to_finish = b;
}

void Base::play()
{
	EngineUtils::set_state(m->pipeline, GST_STATE_PLAYING);
}

void Base::pause()
{
	EngineUtils::set_state(m->pipeline, GST_STATE_PAUSED);
}

void Base::stop()
{
	EngineUtils::set_state(m->pipeline, GST_STATE_NULL);

	m->position_source_ms = 0;
	m->position_pipeline_ms = 0;
	m->duration_ms = 0;
	m->uri = nullptr;
}
