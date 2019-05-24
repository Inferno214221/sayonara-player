/* PlaybackPipeline.cpp */

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

#include "Pipeline.h"
#include "PipelineProbes.h"
#include "Engine.h"

#include "StreamRecorderHandler.h"
#include "Pitcher.h"
#include "Equalizer.h"
#include "Seeker.h"
#include "Broadcaster.h"
#include "Visualizer.h"
#include "EngineUtils.h"

#include "Callbacks/EngineCallbacks.h"
#include "Callbacks/PipelineCallbacks.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Logger/Logger.h"

#include <QTimer>

#include <gst/app/gstappsink.h>

#include <algorithm>
#include <cmath>

using namespace PipelineExtensions;

struct Pipeline::Private
{
	QString				name;
	Engine*				engine=nullptr;
	GstBus*				bus=nullptr;

	GstElement*			pipeline=nullptr;
	GstElement*			source=nullptr;
	GstElement*			audio_convert=nullptr;
	GstElement*			tee=nullptr;

	GstElement*			pb_bin=nullptr;
	GstElement*			pb_queue=nullptr;
	GstElement*			pb_volume=nullptr;
	GstElement*			pb_sink=nullptr;

	Pitcher*		speed_handler=nullptr;
	SeekHandler*		seeker=nullptr;
	StreamRecorderHandler* stream_recorder=nullptr;
	Broadcaster*		broadcaster=nullptr;
	Visualizer*			visualizer=nullptr;
	Equalizer*			equalizer=nullptr;

	QTimer*				progress_timer=nullptr;

	MilliSeconds		duration_ms;
	MilliSeconds		position_source_ms;
	MilliSeconds		position_pipeline_ms;
	bool				about_to_finish;
	bool				initialized;

	Private(const QString& name) :
		name(name),
		duration_ms(0),
		position_source_ms(0),
		position_pipeline_ms(0),
		about_to_finish(false),
		initialized(false)
	{}
};


Pipeline::Pipeline(Engine* engine, const QString& name, QObject *parent) :
	QObject(parent),
	CrossFadeable(),
	Changeable(),
	DelayedPlayable()
{
	m = Pimpl::make<Private>(name);
	m->engine = engine;
}

Pipeline::~Pipeline()
{
	if (m->pipeline)
	{
		EngineUtils::set_state(m->pipeline, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT(m->pipeline));
		m->pipeline = nullptr;
	}
}

bool Pipeline::init(GstState state)
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
	gst_bus_set_sync_handler(m->bus, EngineCallbacks::bus_message_received, m->engine, EngineCallbacks::destroy_notify);
#else
	gst_bus_add_watch(m->bus, EngineCallbacks::bus_state_changed, m->engine);
#endif

	m->progress_timer = new QTimer(this);
	m->progress_timer->setInterval(200);
	connect(m->progress_timer, &QTimer::timeout, this, [=]()
	{
		if(EngineUtils::get_state(m->pipeline) != GST_STATE_NULL){
			PipelineCallbacks::position_changed(this);
		}
	});

	m->progress_timer->start();

	SetSetting(SetNoDB::MP3enc_found, EngineUtils::check_lame_available());
	SetSetting(SetNoDB::Pitch_found, EngineUtils::check_pitch_available());

	ListenSetting(Set::Engine_Vol, Pipeline::s_vol_changed);
	ListenSetting(Set::Engine_Mute, Pipeline::s_mute_changed);
	ListenSettingNoCall(Set::Engine_Sink, Pipeline::s_sink_changed);

	// set by gui, initialized directly in pipeline
	ListenSetting(Set::Engine_ShowLevel, Pipeline::s_show_visualizer_changed);
	ListenSettingNoCall(Set::Engine_ShowSpectrum, Pipeline::s_show_visualizer_changed);
	ListenSettingNoCall(Set::Engine_Pitch, Pipeline::s_speed_changed);
	ListenSettingNoCall(Set::Engine_Speed, Pipeline::s_speed_changed);
	ListenSettingNoCall(Set::Engine_PreservePitch, Pipeline::s_speed_changed);
	ListenSetting(Set::Engine_SpeedActive, Pipeline::s_speed_active_changed);

	m->initialized = true;

	return true;
}


bool Pipeline::create_elements()
{
	// input
	if(!EngineUtils::create_element(&m->audio_convert, "audioconvert")) return false;
	if(!EngineUtils::create_element(&m->tee, "tee")) return false;
	if(!EngineUtils::create_element(&m->pb_queue, "queue", "playback_queue")) return false;
	if(!EngineUtils::create_element(&m->pb_volume, "volume")) return false;

	m->pb_sink = create_sink(GetSetting(Set::Engine_Sink));

	m->seeker = new SeekHandler(m->source);
	m->equalizer = new Equalizer();
	m->speed_handler = new Pitcher();
	m->visualizer = new Visualizer(m->pipeline, m->tee);
	m->broadcaster = new Broadcaster(m->pipeline, m->tee);

	return (m->pb_sink != nullptr);
}


bool Pipeline::create_source(gchar* uri)
{
	if(EngineUtils::create_element(&m->source, "uridecodebin", "src"))
	{
		EngineUtils::set_values(G_OBJECT(m->source),
								"use-buffering", Util::File::is_www(uri),
								"uri", uri);

		EngineUtils::set_uint64_value(G_OBJECT(m->source), "buffer-duration", GetSetting(Set::Engine_BufferSizeMS));

		EngineUtils::add_elements(GST_BIN(m->pipeline), {m->source});
		EngineUtils::set_state(m->source, GST_STATE_NULL);

		g_signal_connect (m->source, "pad-added", G_CALLBACK (PipelineCallbacks::decodebin_ready), m->audio_convert);
		g_signal_connect (m->source, "source-setup", G_CALLBACK (PipelineCallbacks::source_ready), nullptr);
	}

	m->seeker->set_source(m->source);

	return (m->source != nullptr);
}


void Pipeline::remove_source()
{
	if(m->source && EngineUtils::has_element(GST_BIN(m->pipeline), m->source))
	{
		gst_element_send_event(m->pipeline, gst_event_new_eos());
		gst_element_unlink(m->source, m->audio_convert);
		gst_bin_remove(GST_BIN(m->pipeline), m->source);
		m->source = nullptr;
		m->seeker->set_source(nullptr);
	}
}


GstElement* Pipeline::create_sink(const QString& name)
{
	GstElement* ret=nullptr;

	if(name == "pulse"){
		sp_log(Log::Debug, this) << "Create pulseaudio sink";
		EngineUtils::create_element(&ret, "pulsesink", name.toLocal8Bit().data());
	}

	else if(name == "alsa"){
		sp_log(Log::Debug, this) << "Create alsa sink";
		EngineUtils::create_element(&ret, "alsasink", name.toLocal8Bit().data());
	}

	if(ret == nullptr){
		sp_log(Log::Debug, this) << "Create automatic sink";
		EngineUtils::create_element(&ret, "autoaudiosink", name.toLocal8Bit().data());
	}

	return ret;
}


bool Pipeline::add_and_link_elements()
{
	{ // before tee
		gst_bin_add_many(GST_BIN(m->pipeline),
						 m->audio_convert, m->equalizer->element(), m->tee,
						 nullptr);

		bool success = gst_element_link_many(m->audio_convert, m->equalizer->element(), m->tee,  nullptr);
		if(!EngineUtils::test_and_error_bool(success, "Engine: Cannot link audio convert with tee")){
			return false;
		}
	}

	{ // Playback Bin
		m->pb_bin = gst_bin_new("Playback_bin");
		gst_bin_add_many(GST_BIN(m->pb_bin), m->pb_queue, m->pb_volume, m->pb_sink, nullptr);

		bool success = gst_element_link_many(m->pb_queue, m->pb_volume, m->pb_sink, nullptr);
		if(!EngineUtils::test_and_error_bool(success, "Engine: Cannot link eq with audio sink")) {
			return false;
		}

		gst_bin_add(GST_BIN(m->pipeline), m->pb_bin);

		EngineUtils::create_ghost_pad(GST_BIN(m->pb_bin), m->pb_queue);
		EngineUtils::tee_connect(m->tee, m->pb_bin, "Equalizer");

		return EngineUtils::test_and_error_bool(success, "Engine: Cannot link eq queue with tee");
	}
}

void Pipeline::configure_elements()
{
	EngineUtils::set_values(G_OBJECT(m->tee),
				"silent", true,
				"allow-not-linked", true);

	EngineUtils::config_queue(m->pb_queue);
}


bool Pipeline::set_uri(gchar* uri)
{
	stop();
	create_source(uri);
	EngineUtils::set_state(m->pipeline, GST_STATE_PAUSED);

	return true;
}


bool Pipeline::init_streamrecorder()
{
	return m->stream_recorder->init();
}

void Pipeline::play()
{
	EngineUtils::set_state(m->pipeline, GST_STATE_PLAYING);
}

void Pipeline::pause()
{
	EngineUtils::set_state(m->pipeline, GST_STATE_PAUSED);
}


void Pipeline::stop()
{
	EngineUtils::set_state(m->pipeline, GST_STATE_NULL);

	m->position_source_ms = 0;
	m->position_pipeline_ms = 0;
	m->duration_ms = 0;

	abort_delayed_playing();
	abort_fader();

	remove_source();
}

void Pipeline::s_vol_changed()
{
	int vol = GetSetting(Set::Engine_Vol);

	float vol_val = ((vol * 1.0f) / 100.0f);
	EngineUtils::set_value(G_OBJECT(m->pb_volume), "volume", vol_val);
}

void Pipeline::s_mute_changed()
{
	bool muted = GetSetting(Set::Engine_Mute);
	EngineUtils::set_value(G_OBJECT(m->pb_volume), "mute", muted);
}

void Pipeline::enable_visualizer(bool b)
{
	m->visualizer->set_enabled(b);
}

void Pipeline::s_show_visualizer_changed()
{
	enable_visualizer
	(
		GetSetting(Set::Engine_ShowSpectrum) ||
		GetSetting(Set::Engine_ShowLevel)
	);
}

void Pipeline::enable_broadcasting(bool b)
{
	m->broadcaster->set_enabled(b);
}


GstState Pipeline::get_state() const
{
	return EngineUtils::get_state(m->pipeline);
}


MilliSeconds Pipeline::get_time_to_go() const
{
	GstElement* element = m->pipeline;
	MilliSeconds ms = EngineUtils::get_time_to_go(element);
	if(ms < 100){
		return 0;
	}

	return ms - 100;
}


void Pipeline::set_data(Byte* data, uint64_t size)
{
	emit sig_data(data, size);
}

GstElement* Pipeline::pipeline() const
{
	return m->pipeline;
}

void Pipeline::set_equalizer_band(int band, int val)
{
	m->equalizer->set_band(band, val);
}

void Pipeline::enable_streamrecorder(bool b)
{
	m->stream_recorder->set_enabled(b);
}

void Pipeline::set_streamrecorder_path(const QString& path)
{
	m->stream_recorder->set_target_path(path);
}

NanoSeconds Pipeline::seek_rel(double percent, NanoSeconds ref_ns)
{
	return m->seeker->seek_rel(percent, ref_ns);
}

NanoSeconds Pipeline::seek_abs(NanoSeconds ns)
{
	return m->seeker->seek_abs(ns);
}

void Pipeline::set_current_volume(double volume)
{
	EngineUtils::set_value(G_OBJECT(m->pb_volume), "volume", volume);
}

double Pipeline::get_current_volume() const
{
	double volume;
	g_object_get(m->pb_volume, "volume", &volume, nullptr);
	return volume;
}

void Pipeline::s_speed_active_changed()
{
	bool active = GetSetting(Set::Engine_SpeedActive);

	GstElement* speed = m->speed_handler->element();
	if(!speed){
		return;
	}

	MilliSeconds pos_ms = EngineUtils::get_position_ms(m->source);
	if(active) {
		add_element(speed, m->audio_convert, m->equalizer->element());
		s_speed_changed();
	}

	else {
		remove_element(speed, m->audio_convert, m->equalizer->element());
	}

	if(EngineUtils::get_state(m->pipeline) == GST_STATE_PLAYING)
	{
		pos_ms = std::max<MilliSeconds>(pos_ms, 0);
		m->seeker->seek_nearest_ms(pos_ms);
	}
}

void Pipeline::s_speed_changed()
{
	m->speed_handler->set_speed
	(
		GetSetting(Set::Engine_Speed),
		GetSetting(Set::Engine_Pitch) / 440.0,
		GetSetting(Set::Engine_PreservePitch)
	);
}

void Pipeline::s_sink_changed()
{
	GstElement* new_sink = create_sink(GetSetting(Set::Engine_Sink));
	if(!new_sink){
		return;
	}

	MilliSeconds pos_ms = EngineUtils::get_position_ms(m->pipeline);
	GstState old_state = EngineUtils::get_state(m->pipeline);

	{ //replace elements
		gst_element_set_state(m->pipeline, GST_STATE_NULL);

		remove_element(m->pb_sink, m->pb_volume, nullptr);
		add_element(new_sink, m->pb_volume, nullptr);

		gst_element_set_state(m->pipeline, old_state);
	}

	{ //restore position
		if(old_state != GST_STATE_NULL)
		{
			while(old_state != EngineUtils::get_state(m->pipeline)) {
				Util::sleep_ms(50);
			}

			seek_abs(GST_MSECOND * pos_ms);
		}
	}

	m->pb_sink = new_sink;
}

void Pipeline::refresh_position()
{
	GstElement* source = m->source;
	if(!source){
		source = m->pipeline;
	}

	MilliSeconds pos_source_ms = EngineUtils::get_position_ms(source);
	m->position_source_ms = std::max<MilliSeconds>(0, pos_source_ms);

	MilliSeconds pos_pipeline_ms = EngineUtils::get_position_ms(m->pipeline);
	m->position_pipeline_ms = std::max<MilliSeconds>(0, pos_pipeline_ms);

	emit sig_pos_changed_ms( m->position_pipeline_ms );
}

void Pipeline::refresh_duration()
{
	m->duration_ms = EngineUtils::get_duration_ms(m->source);
	refresh_position();
}


void Pipeline::check_about_to_finish()
{
	if(!m->about_to_finish)
	{
		if(m->duration_ms < m->position_pipeline_ms || (m->duration_ms == 0))
		{
			m->duration_ms = EngineUtils::get_duration_ms(m->source);

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


MilliSeconds Pipeline::get_about_to_finish_time() const
{
	MilliSeconds AboutToFinishTime=300;
	return std::max(get_fading_time_ms(), AboutToFinishTime);
}


MilliSeconds Pipeline::get_duration_ms() const
{
	return m->duration_ms;
}

MilliSeconds Pipeline::get_position_ms() const
{
	return m->position_source_ms;
}

bool Pipeline::has_element(GstElement* e) const
{
	return EngineUtils::has_element(GST_BIN(m->pipeline), e);
}

void Pipeline::fade_in_handler()
{
	s_show_visualizer_changed();
}

void Pipeline::fade_out_handler()
{
	enable_visualizer(false);
}
