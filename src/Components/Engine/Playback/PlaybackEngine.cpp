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

#include "PlaybackEngine.h"
#include "PlaybackPipeline.h"
#include "StreamRecorder.h"
#include "Callbacks/EngineUtils.h"
#include "Components/Engine/Callbacks/EngineCallbacks.h"
#include "Components/PlayManager/PlayManager.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/FileUtils.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Logger/Logger.h"

#include <QUrl>

#include <algorithm>
#include <list>

using Engine::Playback;
namespace EngineUtils=Engine::Utils;

/**
 * @brief The GaplessState enum
 * @ingroup Engine
 */
enum class GaplessState : uint8_t
{
	NoGapless=0,		// no gapless enabled at all
	AboutToFinish,		// the phase when the new track is already displayed but not played yet
	TrackFetched,		// track is requested, but no yet there
	Playing,			// currently playing
	Stopped
};

struct Playback::Private
{
	MetaData		md;
	QString			uri;
	MilliSeconds	cur_pos_ms;

	PlayManager*		play_manager=nullptr;
	Pipeline::Playback*	pipeline=nullptr;
	Pipeline::Playback*	other_pipeline=nullptr;

	std::list<LevelReceiver*>		level_receiver;
	std::list<SpectrumReceiver*>	spectrum_receiver;

	StreamRecorder::StreamRecorder*	stream_recorder=nullptr;

	GaplessState gapless_state;
	bool sr_active;

	Private() :
		cur_pos_ms(0),
		gapless_state(GaplessState::Stopped),
		sr_active(false)
	{
		play_manager = PlayManager::instance();
	}

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

Playback::Playback(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}


Playback::~Playback()
{
	if(is_streamrecroder_recording())
	{
		set_streamrecorder_recording(false);
	}
}


bool Playback::init()
{
	gst_init(nullptr, nullptr);

	bool success = init_pipeline(&m->pipeline);
	if(!success){
		return false;
	}

	ListenSetting(Set::Engine_SR_Active, Playback::s_streamrecorder_active_changed);
	ListenSetting(Set::PL_Mode, Playback::s_gapless_changed);
	ListenSetting(Set::Engine_CrossFaderActive, Playback::s_gapless_changed);

	return true;
}

bool Playback::init_pipeline(Pipeline::Playback** pipeline)
{
	if(*pipeline){
		return true;
	}

	*pipeline = new Pipeline::Playback(this);
	Pipeline::Playback* p = *pipeline;

	if(!p->init()){
		m->change_gapless_state(GaplessState::NoGapless);
		return false;
	}

	connect(p, &Pipeline::Playback::sig_about_to_finish, this, &Playback::set_track_almost_finished);
	connect(p, &Pipeline::Playback::sig_pos_changed_ms, this, &Playback::cur_pos_ms_changed);
	connect(p, &Pipeline::Playback::sig_data, this, &Playback::sig_data);

	return true;
}

bool Playback::change_track_crossfading(const MetaData& md)
{
	std::swap(m->pipeline, m->other_pipeline);

	m->other_pipeline->fade_out();

	{
		m->cur_pos_ms = 0;
		bool success = change_metadata(md);
		if (!success) {
			return false;
		}
	}

	m->pipeline->fade_in();

	m->change_gapless_state(GaplessState::Playing);

	return true;
}

bool Playback::change_track_gapless(const MetaData& md)
{
	std::swap(m->pipeline, m->other_pipeline);

	m->cur_pos_ms = 0;
	bool success = change_metadata(md);
	if (!success) {
		return false;
	}

	MilliSeconds time_to_go = m->other_pipeline->get_time_to_go();
	m->pipeline->play_in(time_to_go);

	sp_log(Log::Develop, this) << "Will start playing in " << time_to_go << "msec";

	m->change_gapless_state(GaplessState::TrackFetched);

	return true;
}

bool Playback::change_track_immediatly(const MetaData& md)
{
	if(m->other_pipeline) {
		m->other_pipeline->stop();
	}

	m->pipeline->stop();
	m->cur_pos_ms = 0;

	return change_metadata(md);
}

bool Playback::change_track_by_filename(const QString& filepath)
{
	MetaData md(filepath);

	bool success = true;

	bool got_md = Tagging::Utils::getMetaDataOfFile(md);
	if( !got_md ) {
		stop();
		success = false;
	}

	else
	{
		m->cur_pos_ms = 0;
		success = change_metadata(md);
	}

	return success;
}





bool Playback::change_track(const MetaData& md)
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

bool Playback::change_metadata(const MetaData& md)
{
	bool success = false;
	{
		QString filepath = md.filepath();
		bool playing_stream = Util::File::is_www(filepath);

		// stream, but don't want to record
		// stream is already uri
		if (playing_stream)
		{
			m->uri = QUrl(filepath).toString();
		}

		// no stream (not quite right because of mms, rtsp or other streams
		// normal filepath -> no uri
		else if (!filepath.contains("://"))
		{
			QUrl url = QUrl::fromLocalFile(md.filepath());
			m->uri = url.toString();
		}

		else {
			m->uri = md.filepath();
		}

		if(m->uri.isEmpty())
		{
			m->md = MetaData();

			sp_log(Log::Warning, this) << "uri = 0";
			return false;
		}

		m->md = md;

		success = change_uri(m->uri);
	}

	if(!success)
	{
		m->change_gapless_state(GaplessState::Stopped);
	}

	return success;
}

bool Playback::change_uri(const QString& uri)
{
	return m->pipeline->set_uri(uri.toUtf8().data());
}

void Playback::play()
{
	if( m->gapless_state == GaplessState::AboutToFinish ||
		m->gapless_state == GaplessState::TrackFetched)
	{
		return;
	}

	m->pipeline->play();

	if(is_streamrecroder_recording())
	{
		set_streamrecorder_recording(true);
	}

	m->change_gapless_state(GaplessState::Playing);
}


void Playback::stop()
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
	m->play_manager->buffering(-1);
}


void Playback::pause()
{
	m->pipeline->pause();
}


void Playback::jump_abs_ms(MilliSeconds pos_ms)
{
	m->pipeline->seek_abs(pos_ms * GST_MSECOND);
}

void Playback::jump_rel_ms(MilliSeconds ms)
{
	MilliSeconds new_time_ms = m->pipeline->get_source_position_ms() + ms;
	m->pipeline->seek_abs(new_time_ms * GST_MSECOND);
}


void Playback::jump_rel(double percent)
{
	m->pipeline->seek_rel(percent, m->md.length_ms * GST_MSECOND);
}


void Playback::set_current_position_ms(MilliSeconds pos_ms)
{
	if ( m->cur_pos_ms / 100 == (pos_ms / 100)){
		return;
	}

	m->cur_pos_ms = pos_ms;
	m->play_manager->set_position_ms(pos_ms);
}


MilliSeconds Playback::current_position_ms() const
{
	return m->cur_pos_ms;
}



void Playback::cur_pos_ms_changed(MilliSeconds pos_ms)
{
	if(sender() != m->pipeline){
		return;
	}

	this->set_current_position_ms(pos_ms);
}


void Playback::set_track_ready(GstElement* src)
{
	if(m->pipeline->has_element(src)){
		m->play_manager->set_track_ready();
	}
}

void Playback::set_track_almost_finished(MilliSeconds time2go)
{
	Q_UNUSED(time2go)

	if(sender() != m->pipeline){
		return;
	}

	if( m->gapless_state == GaplessState::NoGapless ||
		m->gapless_state == GaplessState::AboutToFinish )
	{
		emit sig_track_almost_finished(time2go);
		return;
	}

	sp_log(Log::Develop, this) << "About to finish: " <<
								(int) m->gapless_state << " (" << time2go << "ms)";

	m->change_gapless_state(GaplessState::AboutToFinish);

	bool crossfade = GetSetting(Set::Engine_CrossFaderActive);
	if(crossfade) {
		m->pipeline->fade_out();
	}

	m->play_manager->set_track_finished();
}


void Playback::set_track_finished(GstElement* src)
{
	if(m->pipeline->has_element(src))
	{
		m->play_manager->set_track_finished();
	}

	if(m->other_pipeline && m->other_pipeline->has_element(src))
	{
		sp_log(Log::Debug, this) << "Old track finished";

		m->other_pipeline->stop();
		m->change_gapless_state(GaplessState::Playing);
	}
}

bool Playback::is_streamrecroder_recording() const
{
	return (m->sr_active && m->stream_recorder && m->stream_recorder->is_recording());
}


void Playback::set_equalizer(int band, int val)
{
	m->pipeline->set_eq_band(band, val);

	if(m->other_pipeline){
		m->other_pipeline->set_eq_band(band, val);
	}
}


void Playback::set_buffer_state(int progress, GstElement* src)
{
	if(!Util::File::is_www(m->md.filepath())){
		progress = -1;
	}

	if(!m->pipeline->has_element(src)){
		progress = -1;
	}

	m->play_manager->buffering(progress);
}


void Playback::s_gapless_changed()
{
	Playlist::Mode plm = GetSetting(Set::PL_Mode);
	bool gapless =	(Playlist::Mode::isActiveAndEnabled(plm.gapless()) ||
					 GetSetting(Set::Engine_CrossFaderActive));

	if(gapless)
	{
		bool success = init_pipeline(&m->other_pipeline);

		if(success){
			m->change_gapless_state(GaplessState::Stopped);
			return;
		}
	}

	m->change_gapless_state(GaplessState::NoGapless);
}


void Playback::s_streamrecorder_active_changed()
{
	m->sr_active = GetSetting(Set::Engine_SR_Active);

	if(!m->sr_active){
		set_streamrecorder_recording(false);
	}
}


void Playback::set_streamrecorder_recording(bool b)
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

	if(m->pipeline) {
		m->pipeline->set_streamrecorder_path(dst_file);
	}
}

void Playback::set_n_sound_receiver(int num_sound_receiver)
{
	m->pipeline->enable_broadcasting(num_sound_receiver > 0);

	if(m->other_pipeline)
	{
		m->other_pipeline->enable_broadcasting(num_sound_receiver > 0);
	}
}

void Playback::update_cover(const QImage& cover, GstElement* src)
{
	if( m->pipeline->has_element(src) )
	{
		emit sig_cover_changed(cover);
	}
}


void Playback::update_metadata(const MetaData& md, GstElement* src)
{
	if(!m->pipeline->has_element(src)){
		return;
	}

	if(!Util::File::is_www( m->md.filepath() )) {
		return;
	}

	if(md.title().isEmpty()) {
		return;
	}

	QString title = md.title();
	QStringList splitted = md.title().split("-");
	if(splitted.size() == 2) {
		title = splitted[1].trimmed();
	}

	// title is old title
	if(m->md.title().compare(title) == 0) {
		return;
	}

	set_current_position_ms(0);

	MetaData md_update = m->md;
	if(splitted.size() == 2){
		md_update.set_artist(splitted[0].trimmed());
		md_update.set_title(splitted[1].trimmed());
	}

	else {
		md_update.set_title(md.title());
	}

	m->md = md;
	m->play_manager->change_metadata(md);
	emit sig_md_changed(md);

	this->update_metadata(md_update, src);

	if(is_streamrecroder_recording())
	{
		set_streamrecorder_recording(true);
	}
}


void Playback::update_duration(MilliSeconds duration_ms, GstElement* src)
{
	if(! m->pipeline->has_element(src)){
		return;
	}

	m->pipeline->update_duration_ms(duration_ms, src);

	Seconds duration_s = (duration_ms / 1000);
	Seconds md_duration_s = (m->md.length_ms / 1000);

	if(duration_s == 0 || duration_s > 1500000){
		return;
	}

	if(duration_s == md_duration_s) {
		return;
	}

	m->md.length_ms = duration_ms;
	update_metadata(m->md, src);

	m->play_manager->change_duration(m->md.length_ms);
	emit sig_duration_changed(m->md);
}

template<typename T>
T br_diff(T a, T b){ return std::max(a, b) - std::min(a, b); }

void Playback::update_bitrate(Bitrate br, GstElement* src)
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

void Playback::add_spectrum_receiver(SpectrumReceiver* receiver)
{
	m->spectrum_receiver.push_back(receiver);
}

int Playback::get_spectrum_bins() const
{
	return GetSetting(Set::Engine_SpectrumBins);
}

void Playback::set_spectrum(const SpectrumList& vals)
{
	for(SpectrumReceiver* rcv : m->spectrum_receiver)
	{
		if(rcv && rcv->is_active()){
			rcv->set_spectrum(vals);
		}
	}
}

void Playback::add_level_receiver(LevelReceiver* receiver)
{
	m->level_receiver.push_back(receiver);
}

void Playback::set_level(float left, float right)
{
	for(LevelReceiver* rcv : m->level_receiver)
	{
		if(rcv && rcv->is_active()){
			rcv->set_level(left, right);
		}
	}
}


void Playback::error(const QString& error)
{
	QString msg("Cannot play track");

	if(m->md.filepath().contains("soundcloud", Qt::CaseInsensitive))
	{
		msg += QString("\n\n") +
			   "Probably, Sayonara's Soundcloud limit of 15.000 "
			   "tracks per day is reached :( Sorry.";
	}

	if(error.trimmed().length() > 0){
		msg += QString("\n\n") + error;
	}

	m->play_manager->error(error);

	stop();
}
