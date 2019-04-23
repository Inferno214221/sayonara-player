/* PlaybackPipeline.cpp */

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

#include "PlaybackPipeline.h"
#include "PipelineProbes.h"
#include "SpeedHandler.h"
#include "EqualizerHandler.h"
#include "SeekHandler.h"
#include "StreamRecorderHandler.h"
#include "StreamRecorderData.h"
#include "Callbacks/EngineUtils.h"
#include "Callbacks/PipelineCallbacks.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Logger/Logger.h"

#include <gst/app/gstappsink.h>

#include <algorithm>

#define QUEUE "queue"

//http://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/chapter-dataaccess.html

using Pipeline::Playback;
namespace Probing=Pipeline::Probing;
namespace EngineUtils=Engine::Utils;

struct Playback::Private :
		public SpeedHandler,
		public EqualizerHandler,
		public StreamRecorderHandler,
		public SeekHandler
{
	GstElement*			audio_src=nullptr;
	GstElement*			audio_convert=nullptr;
	GstElement*			tee=nullptr;

	GstElement*			pb_bin=nullptr;
	GstElement*			pb_queue=nullptr;
	GstElement*			pb_equalizer=nullptr;
	GstElement*			pb_volume=nullptr;
	GstElement*			pb_pitch=nullptr;
	GstElement*			pb_sink=nullptr;

	GstElement*			visualizer_bin=nullptr;
	GstElement*			visualizer_queue=nullptr;
	GstElement*			spectrum=nullptr;
	GstElement*			level=nullptr;
	GstElement*			visualizer_sink=nullptr;

	GstElement*			bc_bin=nullptr;
	GstElement*			bc_queue=nullptr;
	GstElement*			bc_converter=nullptr;
	GstElement*			bc_resampler=nullptr;
	GstElement*			bc_lame=nullptr;
	GstElement*			bc_app_sink=nullptr;

	GstElement*			sr_bin=nullptr;
	GstElement*			sr_queue=nullptr;
	GstElement*			sr_converter=nullptr;
	GstElement*			sr_sink=nullptr;
	GstElement*			sr_resampler=nullptr;
	GstElement*			sr_lame=nullptr;

	gulong				bc_probe, visualizer_probe;
	bool				run_broadcast, run_visualizer;

	Private() :
		SpeedHandler(),
		EqualizerHandler(),
		StreamRecorderHandler(),
		SeekHandler(),

		bc_probe(0),
		visualizer_probe(0),
		run_broadcast(false),
		run_visualizer(false)
	{}

	protected:
		GstElement* get_pitch_element() const override
		{
			return pb_pitch;
		}

		GstElement* get_equalizer_element() const override
		{
			return pb_equalizer;
		}

		GstElement* get_streamrecorder_sink_element() const override
		{
			return sr_sink;
		}

		GstElement* get_source() const override
		{
			return audio_src;
		}
};


Playback::Playback(Engine::Base* engine, QObject *parent) :
	Pipeline::Base("Playback Pipeline", engine, parent),
	CrossFader(),
	Pipeline::Changeable(),
	Pipeline::DelayedPlayHandler()
{
	m = Pimpl::make<Private>();
}

Playback::~Playback() {}

bool Playback::init(GstState state)
{
	if(!Base::init(state)){
		return false;
	}

	_settings->set<SetNoDB::MP3enc_found>(EngineUtils::check_lame_available());
	_settings->set<SetNoDB::Pitch_found>(EngineUtils::check_pitch_available());

	Set::listen<Set::Engine_Vol>(this, &Playback::s_vol_changed);
	Set::listen<Set::Engine_Mute>(this, &Playback::s_mute_changed);
	Set::listen<Set::Engine_Sink>(this, &Playback::s_sink_changed, false);

	// set by gui, initialized directly in pipeline
	Set::listen<Set::Engine_ShowLevel>(this, &Playback::s_show_visualizer_changed);
	Set::listen<Set::Engine_ShowSpectrum>(this, &Playback::s_show_visualizer_changed, false);
	Set::listen<Set::Engine_Pitch>(this, &Playback::s_speed_changed, false);
	Set::listen<Set::Engine_Speed>(this, &Playback::s_speed_changed, false);
	Set::listen<Set::Engine_PreservePitch>(this, &Playback::s_speed_changed, false);
	Set::listen<Set::Engine_SpeedActive>(this, &Playback::s_speed_active_changed);

	return true;
}


bool Playback::create_elements()
{
	// input
	if(!EngineUtils::create_element(&m->audio_convert, "audioconvert")) return false;
	if(!EngineUtils::create_element(&m->tee, "tee")) return false;

	if(!EngineUtils::create_element(&m->pb_queue, QUEUE, "eq_queue")) return false;
	if(!EngineUtils::create_element(&m->pb_equalizer, "equalizer-10bands")) return false;
	if(!EngineUtils::create_element(&m->pb_volume, "volume")) return false;
	if(!EngineUtils::create_element(&m->pb_pitch, "pitch")){
		m->pb_pitch = nullptr;
	}

	m->pb_sink = create_audio_sink(_settings->get<Set::Engine_Sink>());

	return (m->pb_sink != nullptr);
}


bool Playback::create_source(gchar* uri)
{
	if(EngineUtils::create_element(&m->audio_src, "uridecodebin", "src"))
	{
		g_object_set (G_OBJECT (m->audio_src),
					  "use-buffering", Util::File::is_www(uri),
					  "ring-buffer-max-size", 10000,
					  "buffer-duration", 500 * GST_MSECOND,
					  "uri", uri,
					  nullptr);

		EngineUtils::add_elements(GST_BIN(pipeline()), {m->audio_src});
		EngineUtils::set_state(m->audio_src, GST_STATE_NULL);

		g_signal_connect (m->audio_src, "pad-added", G_CALLBACK (Callbacks::decodebin_ready), m->audio_convert);
		g_signal_connect (m->audio_src, "source-setup", G_CALLBACK (Callbacks::source_ready), nullptr);
	}

	return (m->audio_src != nullptr);
}

void Playback::remove_source()
{
	if(m->audio_src && EngineUtils::has_element(GST_BIN(pipeline()), m->audio_src))
	{
		gst_element_send_event(pipeline(), gst_event_new_eos());
		gst_element_unlink(m->audio_src, m->audio_convert);
		gst_bin_remove(GST_BIN(pipeline()), m->audio_src);
		m->audio_src = nullptr;
	}
}


GstElement* Playback::create_audio_sink(const QString& name)
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


bool Playback::add_and_link_elements()
{
	{ // before tee
		gst_bin_add_many(GST_BIN(pipeline()),
						 m->audio_convert, m->pb_equalizer, m->tee,
						 nullptr);

		bool success = gst_element_link_many(m->audio_convert, m->pb_equalizer, m->tee,  nullptr);
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

		gst_bin_add(GST_BIN(pipeline()), m->pb_bin);

		EngineUtils::create_ghost_pad(GST_BIN(m->pb_bin), m->pb_queue);
		EngineUtils::tee_connect(m->tee, m->pb_bin, "Equalizer");

		return EngineUtils::test_and_error_bool(success, "Engine: Cannot link eq queue with tee");
	}
}

bool Playback::configure_elements()
{
	g_object_set(G_OBJECT(m->tee),
				 "silent", true,
				 "allow-not-linked", true,
				 nullptr);

	m->init_equalizer();
	init_visualizer();

	EngineUtils::config_queue(m->pb_queue);

	return true;
}

bool Playback::init_streamrecorder()
{
	if(m->sr_bin) {
		return true;
	}

	// stream recorder branch
	if(	!EngineUtils::create_element(&m->sr_queue, QUEUE, "sr_queue") ||
		!EngineUtils::create_element(&m->sr_converter, "audioconvert", "sr_converter") ||
		!EngineUtils::create_element(&m->sr_resampler, "audioresample", "sr_resample") ||
		!EngineUtils::create_element(&m->sr_lame, "lamemp3enc", "sr_lame")  ||
		!EngineUtils::create_element(&m->sr_sink, "filesink", "sr_filesink"))
	{
		return false;
	}

	m->streamrecorder_data()->queue = m->sr_queue;
	m->streamrecorder_data()->sink = m->sr_sink;

	{ // configure
		EngineUtils::config_lame(m->sr_lame);
		EngineUtils::config_queue(m->sr_queue);
		EngineUtils::config_sink(m->sr_sink);

		g_object_set(G_OBJECT(m->sr_sink),
					 "buffer-size", 8192,
					 "location", Util::sayonara_path("bla.mp3").toLocal8Bit().data(),
					 nullptr);
	}

	{ // init bin
		bool success = EngineUtils::create_bin(&m->sr_bin, {m->sr_queue, m->sr_converter, m->sr_resampler, m->sr_lame, m->sr_sink}, "sr");
		if(!success){
			return false;
		}

		gst_bin_add(GST_BIN(pipeline()), m->sr_bin);
		success = EngineUtils::tee_connect(m->tee, m->sr_bin, "StreamRecorderQueue");
		if(!success) {
			EngineUtils::set_state(m->sr_bin, GST_STATE_NULL);
			gst_object_unref(m->bc_bin);
		}

		return success;
	}
}


bool Playback::init_broadcasting()
{
	if(m->bc_bin){
		return true;
	}

	// create
	if( !EngineUtils::create_element(&m->bc_queue, QUEUE, "lame_queue") ||
		!EngineUtils::create_element(&m->bc_converter, "audioconvert", "lame_converter") ||
		!EngineUtils::create_element(&m->bc_resampler, "audioresample", "lame_resampler") ||
		!EngineUtils::create_element(&m->bc_lame, "lamemp3enc") ||
		!EngineUtils::create_element(&m->bc_app_sink, "appsink", "lame_appsink"))
	{
		return false;
	}

	{ // init bin
		bool success = EngineUtils::create_bin(&m->bc_bin, {m->bc_queue,  m->bc_converter, m->bc_resampler, m->bc_lame, m->bc_app_sink}, "broadcast");
		if(!success){
			return false;
		}

		gst_bin_add(GST_BIN(pipeline()), m->bc_bin);
		success = EngineUtils::tee_connect(m->tee, m->sr_bin, "StreamRecorderQueue");
		if(!success){
			EngineUtils::set_state(m->bc_bin, GST_STATE_NULL);
			gst_object_unref(m->bc_bin);
			return false;
		}
	}

	{ // configure
		gst_object_ref(m->bc_app_sink);

		EngineUtils::config_lame(m->bc_lame);
		EngineUtils::config_queue(m->bc_queue);
		EngineUtils::config_sink(m->bc_app_sink);

		g_object_set(G_OBJECT(m->bc_app_sink),
					 "emit-signals", true,
					 nullptr );

		g_signal_connect (m->bc_app_sink, "new-sample", G_CALLBACK(Callbacks::new_buffer), this);
	}

	return true;
}

bool Pipeline::Playback::init_visualizer()
{
	if(m->visualizer_bin){
		return true;
	}

	{ // create
		if(	EngineUtils::create_element(&m->visualizer_queue, QUEUE, "visualizer") &&
			EngineUtils::create_element(&m->level, "level") &&	// in case of renaming, also look in EngineCallbase GST_MESSAGE_EVENT
			EngineUtils::create_element(&m->spectrum, "spectrum") &&
			EngineUtils::create_element(&m->visualizer_sink,"fakesink", "visualizer"))
		{
			EngineUtils::create_bin(&m->visualizer_bin, {m->visualizer_queue, m->level, m->spectrum, m->visualizer_sink}, "visualizer");
		}

		if(!m->visualizer_bin){
			return false;
		}
	}

	{ // link
		gst_bin_add(GST_BIN(pipeline()), m->visualizer_bin);
		bool success = EngineUtils::tee_connect(m->tee, m->visualizer_bin, "Visualizer");
		if(!success)
		{
			gst_bin_remove(GST_BIN(pipeline()), m->visualizer_bin);
			gst_object_unref(m->visualizer_bin);
			m->visualizer_bin = nullptr;
			return false;
		}
	}

	{ // configure
		g_object_set (G_OBJECT(m->level),
					  "post-messages", true,
					  "interval", 20 * GST_MSECOND,
					  nullptr);

		g_object_set (G_OBJECT (m->spectrum),
					  "post-messages", true,
					  "interval", 20 * GST_MSECOND,
					  "bands", _settings->get<Set::Engine_SpectrumBins>(),
					  "threshold", -75,
					  "message-phase", false,
					  "message-magnitude", true,
					  "multi-channel", false,
					  nullptr);

		EngineUtils::config_queue(m->visualizer_queue, 1000);
		EngineUtils::config_sink(m->visualizer_sink);
	}

	return true;
}


MilliSeconds Playback::get_about_to_finish_time() const
{
	return std::max( get_fading_time_ms(), Base::get_about_to_finish_time() );
}

void Playback::play()
{
	Base::play();
}

void Playback::stop()
{
	Base::stop();
	abort_delayed_playing();
	abort_fader();

	remove_source();
}

void Playback::s_vol_changed()
{
	int vol = _settings->get<Set::Engine_Vol>();

	float vol_val = (float) ((vol * 1.0f) / 100.0f);

	g_object_set(G_OBJECT(m->pb_volume), "volume", vol_val, nullptr);
}

void Playback::s_mute_changed()
{
	bool muted = _settings->get<Set::Engine_Mute>();
	g_object_set(G_OBJECT(m->pb_volume), "mute", muted, nullptr);
}

void Playback::enable_visualizer(bool b)
{
	if(!init_visualizer()){
		return;
	}

	m->run_visualizer = b;
	Probing::handle_probe(&m->run_visualizer, m->visualizer_queue, &m->visualizer_probe, Probing::spectrum_probed);

	bool show_level = _settings->get<Set::Engine_ShowLevel>();
	bool show_spectrum = _settings->get<Set::Engine_ShowSpectrum>();

	g_object_set(G_OBJECT(m->level), "post-messages", show_level, nullptr);
	g_object_set(G_OBJECT(m->spectrum), "post-messages", show_spectrum, nullptr);
}

void Playback::s_show_visualizer_changed()
{
	enable_visualizer
	(
		_settings->get<Set::Engine_ShowSpectrum>() ||
		_settings->get<Set::Engine_ShowLevel>()
	);
}

void Playback::enable_broadcasting(bool b)
{
	if(b && !init_broadcasting()){
		return;
	}

	m->run_broadcast = b;
	Probing::handle_probe(&m->run_broadcast, m->bc_queue, &m->bc_probe, Probing::lame_probed);
}

GstElement* Playback::get_source() const
{
	return m->audio_src;
}

GstElement* Playback::pipeline() const
{
	// this is needed because of ChangeablePipeline
	return Pipeline::Base::pipeline();
}

bool Playback::set_uri(gchar* uri)
{
	stop();
	create_source(uri);
	gst_element_set_state(pipeline(), GST_STATE_PAUSED);

	return true;
}

void Playback::set_eq_band(int band, int val)
{
	m->set_band(band, val);
}

void Playback::enable_streamrecorder(bool b)
{
	if(b){
		init_streamrecorder();
	}
}

void Playback::set_streamrecorder_path(const QString& path)
{
	m->set_streamrecorder_target_path(path);
}

NanoSeconds Playback::seek_rel(double percent, NanoSeconds ref_ns)
{
	return m->seek_rel(percent, ref_ns);
}

NanoSeconds Playback::seek_abs(NanoSeconds ns)
{
	return m->seek_abs(ns);
}

void Playback::set_current_volume(double volume)
{
	g_object_set(m->pb_volume, "volume", volume, nullptr);
}

double Playback::get_current_volume() const
{
	double volume;
	g_object_get(m->pb_volume, "volume", &volume, nullptr);
	return volume;
}

void Playback::s_speed_active_changed()
{
	if(!m->pb_pitch){
		return;
	}

	MilliSeconds pos_ms = EngineUtils::get_position_ms(get_source());

	bool active = _settings->get<Set::Engine_SpeedActive>();
	if(active) {
		add_element(m->pb_pitch, m->audio_convert, m->pb_equalizer);
		s_speed_changed();
	}

	else {
		remove_element(m->pb_pitch, m->audio_convert, m->pb_equalizer);
	}

	if(this->get_state() == GST_STATE_PLAYING)
	{
		pos_ms = std::max<MilliSeconds>(pos_ms, 0);
		m->seek_nearest_ms(pos_ms);
	}
}

void Playback::s_speed_changed()
{
	if(gst_element_get_parent(m->pb_pitch) == nullptr){
		return;
	}

	m->set_speed(
		_settings->get<Set::Engine_Speed>(),
		_settings->get<Set::Engine_Pitch>() / 440.0,
		_settings->get<Set::Engine_PreservePitch>()
	);
}

void Playback::s_sink_changed()
{
	GstElement* new_sink = create_audio_sink(_settings->get<Set::Engine_Sink>());
	if(!new_sink){
		return;
	}

	MilliSeconds pos_ms = EngineUtils::get_position_ms(pipeline());
	GstState old_state = EngineUtils::get_state(pipeline());

	{ //replace elements
		gst_element_set_state(pipeline(), GST_STATE_NULL);

		remove_element(m->pb_sink, m->pb_volume, nullptr);
		add_element(new_sink, m->pb_volume, nullptr);

		gst_element_set_state(pipeline(), old_state);
	}

	{ //restore position
		if(old_state != GST_STATE_NULL)
		{
			while(old_state != EngineUtils::get_state(pipeline())) {
				Util::sleep_ms(50);
			}

			seek_abs(GST_MSECOND * pos_ms);
		}
	}

	m->pb_sink = new_sink;
}

void Playback::fade_in_handler()
{
	s_show_visualizer_changed();
}

void Playback::fade_out_handler()
{
	enable_visualizer(false);
}

