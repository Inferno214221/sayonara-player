/* PlaybackEngine.cpp */

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

#include "Engine.h"
#include "Components/Engine/Callbacks.h"
#include "Components/Engine/Pipeline.h"
#include "Components/Engine/EngineUtils.h"

#include "StreamRecorder/StreamRecorder.h"

#include "Utils/Macros.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/FileUtils.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"

#include <QUrl>
#include <QList>

#include <algorithm>
#include <exception>

namespace EngineNS=Engine;
using EngineNS::Pipeline;
using EngineNS::PipelinePtr;
using EngineClass=EngineNS::Engine;
namespace EngineUtils=EngineNS::Utils;

struct PipelineCreationException : public std::exception {
   const char* what() const noexcept override;
};

const char* PipelineCreationException::what() const noexcept {
	return "Pipeline could not be created";
}

struct EngineClass::Private
{
	MetaData		md;
	PipelinePtr		pipeline, other_pipeline;

	SpectrumList		spectrum_vals;
	QPair<float, float> level_vals;

	StreamRecorder::StreamRecorder*	stream_recorder=nullptr;

	MilliSeconds	cur_pos_ms;
	GaplessState	gapless_state;

	Private() :
		cur_pos_ms(0),
		gapless_state(GaplessState::Stopped)
	{}

	void change_gapless_state(GaplessState state)
	{
		Playlist::Mode plm = GetSetting(Set::PL_Mode);

		bool gapless = Playlist::Mode::isActiveAndEnabled(plm.gapless());
		bool crossfader = GetSetting(Set::Engine_CrossFaderActive);

		this->gapless_state = state;

		if(!gapless && !crossfader) {
			this->gapless_state = GaplessState::NoGapless;
		}
	}
};

EngineClass::Engine(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	gst_init(nullptr, nullptr);

	GstRegistry* registry = gst_registry_get();
	QString gstreamer_lib_dir = Util::File::clean_filename
	(
		QString(SAYONARA_INSTALL_BIN_PATH) + "/../lib/gstreamer-1.0"
	);

	if(Util::File::exists(gstreamer_lib_dir))
	{
		sp_log(Log::Info, this) << "Scanning for plugins in " << gstreamer_lib_dir;
		gst_registry_scan_path(registry, gstreamer_lib_dir.toLocal8Bit().data());
	}

	m->pipeline = init_pipeline("FirstPipeline");
	if(!m->pipeline) {
		throw PipelineCreationException();
	}

	QString sink = GetSetting(Set::Engine_Sink);
	if(sink == "alsa")
	{
		Playlist::Mode plm = GetSetting(Set::PL_Mode);
		plm.setGapless(false, false);

		SetSetting(Set::Engine_CrossFaderActive, false);
		SetSetting(Set::PL_Mode, plm);
	}

	ListenSetting(Set::Engine_SR_Active, Engine::s_streamrecorder_active_changed);
	ListenSetting(Set::PL_Mode, Engine::s_gapless_changed);
	ListenSetting(Set::Engine_CrossFaderActive, Engine::s_gapless_changed);
}


EngineClass::~Engine()
{
	if(is_streamrecroder_recording())
	{
		set_streamrecorder_recording(false);
	}

	if(m->stream_recorder)
	{
		m->stream_recorder->deleteLater();
		m->stream_recorder = nullptr;
	}
}

PipelinePtr EngineClass::init_pipeline(const QString& name)
{
	PipelinePtr pipeline = std::make_shared<Pipeline>(name);
	if(!pipeline->init(this))
	{
		m->change_gapless_state(GaplessState::NoGapless);
		return nullptr;
	}

	connect(pipeline.get(), &Pipeline::sig_about_to_finish, this, &Engine::set_track_almost_finished);
	connect(pipeline.get(), &Pipeline::sig_pos_changed_ms, this, &Engine::cur_pos_ms_changed);
	connect(pipeline.get(), &Pipeline::sig_data, this, &Engine::sig_data);

	return pipeline;
}

bool EngineClass::change_track_crossfading(const MetaData& md)
{
	std::swap(m->pipeline, m->other_pipeline);

	m->other_pipeline->fade_out();

	bool success = change_metadata(md);
	if (success)
	{
		m->pipeline->fade_in();
		m->change_gapless_state(GaplessState::Playing);
	}

	return success;
}

bool EngineClass::change_track_gapless(const MetaData& md)
{
	std::swap(m->pipeline, m->other_pipeline);

	bool success = change_metadata(md);
	if (success)
	{
		MilliSeconds time_to_go = m->other_pipeline->time_to_go();
		m->pipeline->play_in(time_to_go);

		m->change_gapless_state(GaplessState::TrackFetched);

		sp_log(Log::Develop, this) << "Will start playing in " << time_to_go << "msec";
	}

	return success;
}

bool EngineClass::change_track_immediatly(const MetaData& md)
{
	if(m->other_pipeline) {
		m->other_pipeline->stop();
	}

	m->pipeline->stop();

	return change_metadata(md);
}

bool EngineClass::change_track(const MetaData& md)
{
	if(!m->pipeline)
	{
		return false;
	}

	bool crossfader_active = GetSetting(Set::Engine_CrossFaderActive);
	if(m->gapless_state != GaplessState::Stopped && crossfader_active)
	{
		return change_track_crossfading(md);
	}

	else if(m->gapless_state == GaplessState::AboutToFinish)
	{
		return change_track_gapless(md);
	}

	return change_track_immediatly(md);
}

bool EngineClass::change_metadata(const MetaData& md)
{
	m->md = md;
	set_current_position_ms(0);

	const QString filepath = md.filepath();
	QString uri = filepath;

	bool playing_stream = Util::File::is_www(filepath);
	if (playing_stream)
	{
		uri = QUrl(filepath).toString();
	}

	else if(!filepath.contains("://"))
	{
		QUrl url = QUrl::fromLocalFile(filepath);
		uri = url.toString();
	}

	if(uri.isEmpty())
	{
		m->md = MetaData();

		sp_log(Log::Warning, this) << "uri = 0";
		return false;
	}

	bool success = m->pipeline->prepare(uri);
	if(!success)
	{
		m->change_gapless_state(GaplessState::Stopped);
	}

	return success;
}


void EngineClass::play()
{
	if( m->gapless_state == GaplessState::AboutToFinish ||
		m->gapless_state == GaplessState::TrackFetched)
	{
		return;
	}

	m->pipeline->play();

	if(is_streamrecroder_recording()) {
		set_streamrecorder_recording(true);
	}

	m->change_gapless_state(GaplessState::Playing);
}

void EngineClass::pause()
{
	m->pipeline->pause();
}

void EngineClass::stop()
{
	m->pipeline->stop();

	if(m->other_pipeline){
		m->other_pipeline->stop();
	}

	if(is_streamrecroder_recording()){
		set_streamrecorder_recording(false);
	}

	m->change_gapless_state(GaplessState::Stopped);
	m->cur_pos_ms = 0;
	emit sig_buffering(-1);
}


void EngineClass::jump_abs_ms(MilliSeconds pos_ms)
{
	m->pipeline->seek_abs(pos_ms * GST_MSECOND);
}

void EngineClass::jump_rel_ms(MilliSeconds ms)
{
	MilliSeconds new_time_ms = m->pipeline->position_ms() + ms;
	m->pipeline->seek_abs(new_time_ms * GST_MSECOND);
}

void EngineClass::jump_rel(double percent)
{
	m->pipeline->seek_rel(percent, m->md.duration_ms * GST_MSECOND);
}


void EngineClass::set_current_position_ms(MilliSeconds pos_ms)
{
	if(std::abs(m->cur_pos_ms - pos_ms) >= EngineUtils::get_update_interval())
	{
		m->cur_pos_ms = pos_ms;
		emit sig_current_position_changed(pos_ms);
	}
}

void EngineClass::cur_pos_ms_changed(MilliSeconds pos_ms)
{
	if(sender() == m->pipeline.get()) {
		this->set_current_position_ms(pos_ms);
	}
}


void EngineClass::set_track_ready(GstElement* src)
{
	if(m->pipeline->has_element(src)) {
		emit sig_track_ready();
	}
}

void EngineClass::set_track_almost_finished(MilliSeconds time2go)
{
	if(sender() != m->pipeline.get()){
		return;
	}

	if( m->gapless_state == GaplessState::NoGapless ||
		m->gapless_state == GaplessState::AboutToFinish )
	{
		return;
	}

	sp_log(Log::Develop, this) << "About to finish: " <<
		int(m->gapless_state) << " (" << time2go << "ms)";

	m->change_gapless_state(GaplessState::AboutToFinish);

	bool crossfade = GetSetting(Set::Engine_CrossFaderActive);
	if(crossfade) {
		m->pipeline->fade_out();
	}

	emit sig_track_finished();
}

void EngineClass::set_track_finished(GstElement* src)
{
	if(m->pipeline->has_element(src)) {
		emit sig_track_finished();
	}

	if(m->other_pipeline && m->other_pipeline->has_element(src))
	{
		sp_log(Log::Debug, this) << "Old track finished";

		m->other_pipeline->stop();
		m->change_gapless_state(GaplessState::Playing);
	}
}

void EngineClass::set_equalizer(int band, int val)
{
	m->pipeline->set_equalizer_band(band, val);

	if(m->other_pipeline){
		m->other_pipeline->set_equalizer_band(band, val);
	}
}

MetaData Engine::Engine::current_track() const
{
	return m->md;
}

void EngineClass::set_buffer_state(int progress, GstElement* src)
{
	if(!Util::File::is_www(m->md.filepath())){
		progress = -1;
	}

	else if(!m->pipeline->has_element(src)){
		progress = -1;
	}

	emit sig_buffering(progress);
}

void EngineClass::s_gapless_changed()
{
	Playlist::Mode plm = GetSetting(Set::PL_Mode);
	bool gapless =	(Playlist::Mode::isActiveAndEnabled(plm.gapless()) ||
					 GetSetting(Set::Engine_CrossFaderActive));

	if(gapless)
	{
		if(!m->other_pipeline) {
			m->other_pipeline = init_pipeline("SecondPipeline");
		}

		m->change_gapless_state(GaplessState::Stopped);
	}

	else {
		m->change_gapless_state(GaplessState::NoGapless);
	}
}


void EngineClass::s_streamrecorder_active_changed()
{
	bool is_active = GetSetting(Set::Engine_SR_Active);
	if(!is_active) {
		set_streamrecorder_recording(false);
	}
}

bool EngineClass::is_streamrecroder_recording() const
{
	bool sr_active = GetSetting(Set::Engine_SR_Active);
	return (sr_active && m->stream_recorder && m->stream_recorder->is_recording());
}

void EngineClass::set_streamrecorder_recording(bool b)
{
	if(b)
	{
		if(!m->stream_recorder) {
			m->stream_recorder = new StreamRecorder::StreamRecorder(this);
		}
	}

	if(!m->stream_recorder)	{
		return;
	}

	m->pipeline->record(b);

	if(m->stream_recorder->is_recording() != b){
		m->stream_recorder->record(b);
	}

	QString dst_file;
	if(b)
	{
		dst_file = m->stream_recorder->change_track(m->md);
		if(dst_file.isEmpty()){
			return;
		}
	}

	m->pipeline->set_recording_path(dst_file);
}


void EngineClass::update_cover(GstElement* src, const QByteArray& data, const QString& mimetype)
{	
	if(m->pipeline->has_element(src))
	{
		emit sig_cover_data(data, mimetype);
	}
}

void EngineClass::update_metadata(const MetaData& md, GstElement* src)
{
	if(!m->pipeline->has_element(src)){
		return;
	}

	if(!Util::File::is_www( m->md.filepath() )) {
		return;
	}

	m->md = md;

	set_current_position_ms(0);

	emit sig_metadata_changed(m->md);

	if(is_streamrecroder_recording())
	{
		set_streamrecorder_recording(true);
	}
}

void EngineClass::update_duration(GstElement* src)
{
	if(!m->pipeline->has_element(src)){
		return;
	}

	MilliSeconds duration_ms = m->pipeline->duration_ms();
	MilliSeconds difference = std::abs(duration_ms - m->md.duration_ms);
	if(duration_ms < 1000 || difference < 1999 || duration_ms > 1500000000){
		return;
	}

	m->md.duration_ms = duration_ms;
	update_metadata(m->md, src);

	emit sig_duration_changed(m->md);

	m->pipeline->check_position();
}

template<typename T>
T br_diff(T a, T b){ return std::max(a, b) - std::min(a, b); }

void EngineClass::update_bitrate(Bitrate bitrate, GstElement* src)
{
	if( (!m->pipeline->has_element(src)) ||
		(bitrate == 0) ||
		(br_diff(bitrate, m->md.bitrate) < 1000) )
	{
		return;
	}

	m->md.bitrate = bitrate;

	emit sig_bitrate_changed(m->md);
}

void EngineClass::set_broadcast_enabled(bool b)
{
	m->pipeline->enable_broadcasting(b);
	if(m->other_pipeline) {
		m->other_pipeline->enable_broadcasting(b);
	}
}

void EngineClass::set_spectrum(const SpectrumList& vals)
{
	m->spectrum_vals = vals;
	emit sig_spectrum_changed();
}

Engine::SpectrumList Engine::Engine::spectrum() const
{
	return m->spectrum_vals;
}

void EngineClass::set_level(float left, float right)
{
	m->level_vals = {left, right};
	emit sig_level_changed();
}

QPair<float, float> Engine::Engine::level() const
{
	return m->level_vals;
}


void EngineClass::error(const QString& error, const QString& element_name)
{
	QStringList msg{Lang::get(Lang::Error)};

	if(m->md.filepath().contains("soundcloud", Qt::CaseInsensitive))
	{
		msg << "Probably, Sayonara's Soundcloud limit of 15.000 "
			   "tracks per day is reached :( Sorry.";
	}

	if(error.trimmed().length() > 0){
		msg << error;
	}

	if(element_name.contains("alsa"))
	{
		msg << tr("You should restart Sayonara now") + ".";
	}

	stop();

	emit sig_error(msg.join("\n\n"));
}
