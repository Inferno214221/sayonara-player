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

#include "Callbacks/EngineCallbacks.h"
#include "Callbacks/PipelineCallbacks.h"
#include "Utils/Logger/Logger.h"

#include <QTimer>
#include <gst/app/gstappsink.h>

using Pipeline::Base;
using Pipeline::test_and_error;
using Pipeline::test_and_error_bool;

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

	MilliSeconds query_duration(GstElement* source) const
	{
		NanoSeconds duration_ns;
		bool success = gst_element_query_duration(source, GST_FORMAT_TIME, &duration_ns);

		if(success && (duration_ns >= 0)){
			return (MilliSeconds) (GST_TIME_AS_MSECONDS(duration_ns));
		}

		return 0;
	}
};

Base::Base(QString name, Engine::Base* engine, QObject* parent) :
	QObject(parent),
	SayonaraClass()
{
	m = Pimpl::make<Private>(name, engine);
}

Base::~Base()
{
	if (m->pipeline) {
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
	if(!test_and_error(m->pipeline, "Engine: Pipeline sucks")){
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

	gst_element_set_state(m->pipeline, state);

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
	NanoSeconds pos_pipeline, pos_source;
	bool success_source, success_pipeline;
	GstElement* element;

	element = get_source();

	if(!element){
		element = GST_ELEMENT(m->pipeline);
	}

	success_source = gst_element_query_position(element, GST_FORMAT_TIME, &pos_source);
	success_pipeline = gst_element_query_position(m->pipeline, GST_FORMAT_TIME, &pos_pipeline);

	m->position_source_ms = 0;
	m->position_pipeline_ms = 0;
	if(success_source) {
		m->position_source_ms = (MilliSeconds) (GST_TIME_AS_MSECONDS(pos_source));
	}

	if(success_pipeline) {
		m->position_pipeline_ms = (MilliSeconds) (GST_TIME_AS_MSECONDS(pos_pipeline));
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
			m->duration_ms = m->query_duration(get_source());

			if(m->duration_ms == 0){
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
	NanoSeconds position, duration;

	GstElement* element = get_source();
	if(!element){
		element = GST_ELEMENT(m->pipeline);
	}

	element = GST_ELEMENT(m->pipeline);

	gst_element_query_duration(element, GST_FORMAT_TIME, &duration);
	gst_element_query_position(element, GST_FORMAT_TIME, &position);

	if(duration < position) {
		return 0;
	}

	else {
		return (MilliSeconds) (GST_TIME_AS_MSECONDS(duration - position) - 100);
	}
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

void Base::finished()
{
	emit sig_finished();
}


GstState Base::get_state()
{
	GstState state;
	gst_element_get_state(m->pipeline, &state, nullptr, GST_MSECOND * 10);
	return state;
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



bool Base::create_element(GstElement** elem, const gchar* elem_name, const gchar* name)
{
	QString error_msg;
	if(strlen(name) > 0){
		*elem = gst_element_factory_make(elem_name, name);
		error_msg = QString("Engine: ") + name + " creation failed";
	}

	else{
		*elem = gst_element_factory_make(elem_name, elem_name);
		error_msg = QString("Engine: ") + elem_name + " creation failed";
	}

	bool success = test_and_error(*elem, error_msg);
	m->elements << *elem;

	return success;
}


bool Base::tee_connect(GstElement* tee, GstElement* queue, const QString& queue_name)
{
	GstPadLinkReturn s;
	GstPad* tee_queue_pad;
	GstPad* queue_pad;

	QString error_1 = QString("Engine: Tee-") + queue_name + " pad is nullptr";
	QString error_2 = QString("Engine: ") + queue_name + " pad is nullptr";
	QString error_3 = QString("Engine: Cannot link tee with ") + queue_name;

	GstPadTemplate* tee_src_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(tee), "src_%u");
	if(!test_and_error(tee_src_pad_template, "Engine: _tee_src_pad_template is nullptr")) {
		return false;
	}

	tee_queue_pad = gst_element_request_pad(tee, tee_src_pad_template, nullptr, nullptr);
	if(!test_and_error(tee_queue_pad, error_1)){
		return false;
	}

	queue_pad = gst_element_get_static_pad(queue, "sink");
	if(!test_and_error(queue_pad, error_2)) {
		return false;
	}

	s = gst_pad_link (tee_queue_pad, queue_pad);
	if(!test_and_error_bool((s == GST_PAD_LINK_OK), error_3)) {
		return false;
	}

	g_object_set (queue,
		"silent", true,
		"flush-on-eos", true,
		"max-size-bytes", 1000000,
		"max-size-time", 500000000,
		nullptr);

	//g_object_set(queue, "ring-buffer-max-size", 1048576 , nullptr);

	gst_object_unref(tee_queue_pad);
	gst_object_unref(queue_pad);
	return true;
}


bool
Base::has_element(GstElement* e) const
{
	if(!e){
		return true;
	}

	GstObject* o = GST_OBJECT(e);
	GstObject* parent = nullptr;

	while(o)
	{
		if( o == GST_OBJECT(m->pipeline))
		{
			if( o != GST_OBJECT(e) )
			{
				gst_object_unref(o);
			}

			return true;
		}

		parent = gst_object_get_parent(o);
		if( o != GST_OBJECT(e) )
		{
			gst_object_unref(o);
		}

		o = parent;
	}

	return false;
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
	if(m->pipeline)
	{
		gst_element_set_state(m->pipeline, GST_STATE_PLAYING);
	}
}

void Base::pause()
{
	if(m->pipeline)
	{
		gst_element_set_state(m->pipeline, GST_STATE_PAUSED);
	}
}

void Base::stop()
{
	if(m->pipeline)
	{
		gst_element_set_state(m->pipeline, GST_STATE_NULL);
	}

	m->position_source_ms = 0;
	m->position_pipeline_ms = 0;
	m->duration_ms = 0;
	m->uri = nullptr;
}

bool
Pipeline::test_and_error(void* element, const QString& errorstr)
{
	if(!element) {
		sp_log(Log::Error) << errorstr;
		return false;
	}

	return true;
}

bool
Pipeline::test_and_error_bool(bool b, const QString& errorstr)
{
	if(!b) {
		sp_log(Log::Error) << errorstr;
		return false;
	}

	return true;
}
