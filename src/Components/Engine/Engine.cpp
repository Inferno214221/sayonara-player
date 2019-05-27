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
#include "Components/Engine/Utils.h"

#include "StreamRecorder/StreamRecorder.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/FileUtils.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"

#include <QUrl>
#include <QList>

#include <algorithm>

using EngineImpl=Engine::Engine;

struct EngineImpl::Private
{
	MetaData		md;
	MilliSeconds	cur_pos_ms;

	Pipeline*		pipeline=nullptr;
	Pipeline*		other_pipeline=nullptr;

	QList<LevelReceiver*>		level_receiver;
	QList<SpectrumReceiver*>	spectrum_receiver;

	StreamRecorder::StreamRecorder*	stream_recorder=nullptr;

	GaplessState gapless_state;

	Private() :
		cur_pos_ms(0),
		gapless_state(GaplessState::Stopped)
	{}

	~Private()
	{
		delete pipeline; pipeline = nullptr;
		if(other_pipeline)
		{
			delete other_pipeline; other_pipeline = nullptr;
		}

		if(stream_recorder){
			stream_recorder->deleteLater();
		}
	}

	void change_gapless_state(GaplessState state)
	{
		Playlist::Mode plm = GetSetting(Set::PL_Mode);

		bool gapless = Playlist::Mode::isActiveAndEnabled(plm.gapless());
		bool crossfader = GetSetting(Set::Engine_CrossFaderActive);

		gapless_state = state;

		if(!gapless && !crossfader) {
			gapless_state = GaplessState::NoGapless;
		}
	}
};

EngineImpl::Engine(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}


EngineImpl::~Engine()
{
	if(is_streamrecroder_recording())
	{
		set_streamrecorder_recording(false);
	}
}


bool EngineImpl::init()
{
	gst_init(nullptr, nullptr);

	bool success = init_pipeline(&m->pipeline, "FirstPipeline");
	if(!success){
		return false;
	}

	ListenSetting(Set::Engine_SR_Active, Engine::s_streamrecorder_active_changed);
	ListenSetting(Set::PL_Mode, Engine::s_gapless_changed);
	ListenSetting(Set::Engine_CrossFaderActive, Engine::s_gapless_changed);

	return true;
}

bool EngineImpl::init_pipeline(Pipeline** pipeline, const QString& name)
{
	if(*pipeline){
		return true;
	}

	*pipeline = new Pipeline(name);
	Pipeline* p = *pipeline;

	if(!p->init(this)){
		m->change_gapless_state(GaplessState::NoGapless);
		return false;
	}

	connect(p, &Pipeline::sig_about_to_finish, this, &Engine::set_track_almost_finished);
	connect(p, &Pipeline::sig_pos_changed_ms, this, &Engine::cur_pos_ms_changed);
	connect(p, &Pipeline::sig_data, this, &Engine::sig_data);

	return true;
}

bool EngineImpl::change_track_crossfading(const MetaData& md)
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

bool EngineImpl::change_track_gapless(const MetaData& md)
{
	std::swap(m->pipeline, m->other_pipeline);

	bool success = change_metadata(md);
	if (success)
	{
		MilliSeconds time_to_go = m->other_pipeline->get_time_to_go();
		m->pipeline->play_in(time_to_go);

		m->change_gapless_state(GaplessState::TrackFetched);

		sp_log(Log::Develop, this) << "Will start playing in " << time_to_go << "msec";
	}

	return success;
}

bool EngineImpl::change_track_immediatly(const MetaData& md)
{
	if(m->other_pipeline) {
		m->other_pipeline->stop();
	}

	m->pipeline->stop();

	return change_metadata(md);
}

bool EngineImpl::change_track(const MetaData& md)
{
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

bool EngineImpl::change_metadata(const MetaData& md)
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

	bool success = m->pipeline->set_uri(uri.toUtf8().data());
	if(!success)
	{
		m->change_gapless_state(GaplessState::Stopped);
	}

	return success;
}

void EngineImpl::play()
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


void EngineImpl::stop()
{
	m->change_gapless_state(GaplessState::Stopped);

	sp_log(Log::Info, this) << "Stop";
	m->pipeline->stop();

	if(m->other_pipeline){
		m->other_pipeline->stop();
	}

	if(is_streamrecroder_recording()){
		set_streamrecorder_recording(false);
	}

	m->cur_pos_ms = 0;
	emit sig_buffering(-1);
}


void EngineImpl::pause()
{
	m->pipeline->pause();
}


void EngineImpl::jump_abs_ms(MilliSeconds pos_ms)
{
	m->pipeline->seek_abs(pos_ms * GST_MSECOND);
}

void EngineImpl::jump_rel_ms(MilliSeconds ms)
{
	MilliSeconds new_time_ms = m->pipeline->get_position_ms() + ms;
	m->pipeline->seek_abs(new_time_ms * GST_MSECOND);
}


void EngineImpl::jump_rel(double percent)
{
	m->pipeline->seek_rel(percent, m->md.length_ms * GST_MSECOND);
}


void EngineImpl::set_current_position_ms(MilliSeconds pos_ms)
{
	if(std::abs(m->cur_pos_ms - pos_ms) < 100)
	{
		return;
	}

	m->cur_pos_ms = pos_ms;

	emit sig_current_position_changed(pos_ms);
}


void EngineImpl::cur_pos_ms_changed(MilliSeconds pos_ms)
{
	if(sender() != m->pipeline){
		return;
	}

	this->set_current_position_ms(pos_ms);
}


void EngineImpl::set_track_ready(GstElement* src)
{
	if(m->pipeline->has_element(src)){
		emit sig_track_ready();
	}
}

void EngineImpl::set_track_almost_finished(MilliSeconds time2go)
{
	Q_UNUSED(time2go)

	if(sender() != m->pipeline){
		return;
	}

	if( m->gapless_state == GaplessState::NoGapless ||
		m->gapless_state == GaplessState::AboutToFinish )
	{
		return;
	}

	sp_log(Log::Develop, this) << "About to finish: " <<
		static_cast<int>(m->gapless_state) << " (" << time2go << "ms)";

	m->change_gapless_state(GaplessState::AboutToFinish);

	bool crossfade = GetSetting(Set::Engine_CrossFaderActive);
	if(crossfade) {
		m->pipeline->fade_out();
	}

	emit sig_track_finished();
}


void EngineImpl::set_track_finished(GstElement* src)
{
	if(m->pipeline->has_element(src))
	{
		emit sig_track_finished();
	}

	if(m->other_pipeline && m->other_pipeline->has_element(src))
	{
		sp_log(Log::Debug, this) << "Old track finished";

		m->other_pipeline->stop();
		m->change_gapless_state(GaplessState::Playing);
	}
}

bool EngineImpl::is_streamrecroder_recording() const
{
	bool sr_active = GetSetting(Set::Engine_SR_Active);
	return (sr_active && m->stream_recorder && m->stream_recorder->is_recording());
}


void EngineImpl::set_equalizer(int band, int val)
{
	m->pipeline->set_equalizer_band(band, val);

	if(m->other_pipeline){
		m->other_pipeline->set_equalizer_band(band, val);
	}
}


void EngineImpl::set_buffer_state(int progress, GstElement* src)
{
	if(!Util::File::is_www(m->md.filepath())){
		progress = -1;
	}

	else if(!m->pipeline->has_element(src)){
		progress = -1;
	}

	emit sig_buffering(progress);
}


void EngineImpl::s_gapless_changed()
{
	Playlist::Mode plm = GetSetting(Set::PL_Mode);
	bool gapless =	(Playlist::Mode::isActiveAndEnabled(plm.gapless()) ||
					 GetSetting(Set::Engine_CrossFaderActive));

	if(gapless)
	{
		bool success = init_pipeline(&m->other_pipeline, "SecondPipeline");

		if(success){
			m->change_gapless_state(GaplessState::Stopped);
			return;
		}
	}

	m->change_gapless_state(GaplessState::NoGapless);
}


void EngineImpl::s_streamrecorder_active_changed()
{
	bool is_active = GetSetting(Set::Engine_SR_Active);
	if(!is_active){
		set_streamrecorder_recording(false);
	}
}


void EngineImpl::set_streamrecorder_recording(bool b)
{
	if(b)
	{
		if(m->pipeline) {
			m->pipeline->enable_streamrecorder(b);
		}

		if(!m->stream_recorder) {
			m->stream_recorder = new StreamRecorder::StreamRecorder(this);
		}
	}

	if(!m->stream_recorder)	{
		return;
	}

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

	if(m->pipeline)
	{
		m->pipeline->set_streamrecorder_path(dst_file);
	}
}

void EngineImpl::set_n_sound_receiver(int num_sound_receiver)
{
	m->pipeline->enable_broadcasting(num_sound_receiver > 0);

	if(m->other_pipeline)
	{
		m->other_pipeline->enable_broadcasting(num_sound_receiver > 0);
	}
}

void EngineImpl::update_cover(const QImage& cover, GstElement* src)
{
	if( m->pipeline->has_element(src) )
	{
		emit sig_cover_changed(cover);
	}
}


void EngineImpl::update_metadata(const MetaData& md, GstElement* src)
{
	if(!m->pipeline->has_element(src)){
		return;
	}

	if(!Util::File::is_www( m->md.filepath() )) {
		return;
	}

	QString title = md.title();

	QStringList splitted = title.split("-");
	if(splitted.size() == 2) {
		title = splitted[1].trimmed();
	}

	if(title.isEmpty() || title == m->md.title()) {
		return;
	}

	set_current_position_ms(0);

	if(splitted.size() == 2)
	{
		m->md.set_artist(splitted[0].trimmed());
		m->md.set_title(splitted[1].trimmed());
	}

	else {
		m->md.set_title(md.title());
	}

	emit sig_md_changed(m->md);

	if(is_streamrecroder_recording())
	{
		set_streamrecorder_recording(true);
	}
}


void EngineImpl::update_duration(MilliSeconds duration_ms, GstElement* src)
{
	if(! m->pipeline->has_element(src)){
		return;
	}

	m->pipeline->refresh_duration();

	MilliSeconds difference = std::abs(duration_ms - m->md.length_ms);
	if(duration_ms < 1000 || difference < 1999 || duration_ms > 1500000000){
		return;
	}

	m->md.length_ms = duration_ms;
	update_metadata(m->md, src);

	emit sig_duration_changed(m->md);
}

template<typename T>
T br_diff(T a, T b){ return std::max(a, b) - std::min(a, b); }

void EngineImpl::update_bitrate(Bitrate br, GstElement* src)
{
	if( (br <= 0) ||
		(!m->pipeline->has_element(src)) ||
		(br_diff(br, m->md.bitrate) < 1000) )
	{
		return;
	}

	m->md.bitrate = br;

	emit sig_bitrate_changed(m->md);
}

void EngineImpl::add_spectrum_receiver(SpectrumReceiver* receiver)
{
	m->spectrum_receiver.push_back(receiver);
}

void EngineImpl::set_spectrum(const SpectrumList& vals)
{
	for(SpectrumReceiver* rcv : m->spectrum_receiver)
	{
		if(rcv && rcv->is_active()){
			rcv->set_spectrum(vals);
		}
	}
}

void EngineImpl::add_level_receiver(LevelReceiver* receiver)
{
	m->level_receiver.push_back(receiver);
}

void EngineImpl::set_level(float left, float right)
{
	for(LevelReceiver* rcv : m->level_receiver)
	{
		if(rcv && rcv->is_active()){
			rcv->set_level(left, right);
		}
	}
}


void EngineImpl::error(const QString& error)
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

	stop();

	emit sig_error(msg.join("\n\n"));
}
