/* PlayManager.cpp */

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

#include "PlayManager.h"
#include "Interfaces/Notification/NotificationHandler.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"

#include <QDateTime>
#include <QTime>

#include <algorithm>
#include <array>

template<typename T, int N_ITEMS>
class RingBuffer
{
private:
	int _cur_idx;
	int _n_items;
	std::array<T, N_ITEMS> _data;

public:
	RingBuffer()
	{
		clear();
	}

	void clear()
	{
		_cur_idx = 0;
		_n_items = 0;
	}

	void insert(const T& item)
	{
		_data[_cur_idx] = item;
		_cur_idx = (_cur_idx + 1) % N_ITEMS;
		_n_items = std::min(N_ITEMS, _n_items + 1);
	}

	bool has_item(const T& item) const
	{
		auto it = std::find(_data.begin(), _data.end(), item);
		return (it != _data.end());
	}

	int count() const
	{
		return _n_items;
	}

	bool is_empty() const
	{
		return (count() == 0);
	}
};


struct PlayManager::Private
{
	MetaData				md;
	RingBuffer<QString, 3>	ring_buffer;
	int						track_idx;
	MilliSeconds			position_ms;
	MilliSeconds			initial_position_ms;
	PlayState				playstate;

	Private()
	{
		reset();
		playstate = PlayState::FirstStartup;
	}

	void reset()
	{
		md = MetaData();
		ring_buffer.clear();
		track_idx = -1;
		position_ms = 0;
		initial_position_ms = 0;
		playstate = PlayState::Stopped;
	}
};


PlayManager::PlayManager(QObject* parent) :
	QObject(parent),
	SayonaraClass()
{
	m = Pimpl::make<Private>();

	bool load_playlist = (_settings->get<Set::PL_LoadSavedPlaylists>() || _settings->get<Set::PL_LoadTemporaryPlaylists>());
	bool load_last_track = _settings->get<Set::PL_LoadLastTrack>();
	bool remember_last_time = _settings->get<Set::PL_RememberTime>();

	if(	load_playlist &&
			load_last_track &&
			remember_last_time)
	{
		m->initial_position_ms = _settings->get<Set::Engine_CurTrackPos_s>() * 1000;
	}

	else {
		m->initial_position_ms = 0;
	}
}

PlayManager::~PlayManager() {}

PlayState PlayManager::playstate() const
{
	return m->playstate;
}

MilliSeconds PlayManager::current_position_ms() const
{
	return m->position_ms;
}

MilliSeconds PlayManager::initial_position_ms() const
{
	return m->initial_position_ms;
}

MilliSeconds PlayManager::duration_ms() const
{
	return m->md.length_ms;
}

const MetaData& PlayManager::current_track() const
{
	return m->md;
}

int PlayManager::volume() const
{
	return _settings->get<Set::Engine_Vol>();
}

bool PlayManager::is_muted() const
{
	return _settings->get<Set::Engine_Mute>();
}


void PlayManager::play()
{
	m->playstate = PlayState::Playing;
	emit sig_playstate_changed(m->playstate);
}

void PlayManager::wake_up()
{
	emit sig_wake_up();
}

void PlayManager::play_pause()
{
	if(m->playstate == PlayState::Playing) {
		pause();
	}

	else if(m->playstate == PlayState::Stopped) {
		wake_up();
	}

	else {
		play();
	}
}


void PlayManager::pause()
{
	m->playstate = PlayState::Paused;
	emit sig_playstate_changed(m->playstate);
}


void PlayManager::previous()
{
	emit sig_previous();
}


void PlayManager::next()
{
	emit sig_next();
}


void PlayManager::stop()
{
	m->reset();

	emit sig_playstate_changed(m->playstate);
}


void PlayManager::record(bool b)
{
	if(_settings->get<SetNoDB::MP3enc_found>()) {
		emit sig_record(b);
	} else {
		emit sig_record(false);
	}
}

void PlayManager::seek_rel(double percent)
{
	emit sig_seeked_rel(percent);
}

void PlayManager::seek_rel_ms(MilliSeconds ms)
{
	emit sig_seeked_rel_ms(ms);
}

void PlayManager::seek_abs_ms(MilliSeconds ms)
{
	emit sig_seeked_abs_ms(ms);
}

void PlayManager::set_position_ms(MilliSeconds ms)
{
	m->position_ms = ms;

	_settings->set<Set::Engine_CurTrackPos_s>((int) (m->position_ms / 1000));

	emit sig_position_changed_ms(ms);
}


void PlayManager::change_track(const MetaData& md, int track_idx)
{
	m->md = md;
	m->position_ms = 0;
	m->track_idx = track_idx;
	m->ring_buffer.clear();

	// initial position is outdated now and never needed again
	if(m->initial_position_ms > 0)
	{
		int old_idx = _settings->get<Set::PL_LastTrack>();
		if(old_idx != m->track_idx) {
			m->initial_position_ms = 0;
		}
	}

	// play or stop
	if(m->track_idx >= 0)
	{
		emit sig_track_changed(m->md);
		emit sig_track_idx_changed(m->track_idx);

		play();

		if( (md.radio_mode() != RadioMode::Off) &&
				_settings->get<Set::Engine_SR_Active>() &&
				_settings->get<Set::Engine_SR_AutoRecord>() )
		{
			record(true);
		}
	}

	else {
		sp_log(Log::Info, this) << "Playlist finished";
		emit sig_playlist_finished();
		stop();
	}

	// save last track
	if(md.db_id() == 0) {
		_settings->set<Set::PL_LastTrack>(m->track_idx);
	}

	else{
		_settings->set<Set::PL_LastTrack>(-1);
	}

	// show notification
	if(_settings->get<Set::Notification_Show>()) {
		if(m->track_idx > -1 && !m->md.filepath().isEmpty()) {
			NotificationHandler::instance()->notify(m->md);
		}
	}
}

void PlayManager::set_track_ready()
{
	if(m->initial_position_ms == 0) {
		return;
	}

	sp_log(Log::Debug, this) << "Track ready, " << m->initial_position_ms / 1000;
	this->seek_abs_ms(m->initial_position_ms);

	m->initial_position_ms = 0;

	if(_settings->get<Set::PL_StartPlaying>()){
		play();
	}

	else {
		pause();
	}
}

void PlayManager::buffering(int progress)
{
	emit sig_buffer(progress);
}


void PlayManager::volume_up()
{
	set_volume(_settings->get<Set::Engine_Vol>() + 5);
}

void PlayManager::volume_down()
{
	set_volume(_settings->get<Set::Engine_Vol>() - 5);
}

void PlayManager::set_volume(int vol)
{
	vol = std::min(vol, 100);
	vol = std::max(vol, 0);
	_settings->set<Set::Engine_Vol>(vol);
	emit sig_volume_changed(vol);
}

void PlayManager::set_muted(bool b)
{
	_settings->set<Set::Engine_Mute>(b);
	emit sig_mute_changed(b);
}

void PlayManager::toggle_mute()
{
	bool muted = _settings->get<Set::Engine_Mute>();
	set_muted(!muted);
}

void PlayManager::change_duration(MilliSeconds ms)
{
	m->md.length_ms = ms;

	emit sig_duration_changed(ms);
}

void PlayManager::error(const QString& message)
{
	emit sig_error(message);
}

void PlayManager::change_metadata(const MetaData& md)
{
	MetaData md_old = m->md;
	m->md = md;

	QString str = md.title() + md.artist() + md.album();
	bool has_data = m->ring_buffer.has_item(str);

	if(!has_data)
	{
		if(_settings->get<Set::Notification_Show>()) {
			NotificationHandler::instance()->notify(m->md);
		}

		if( m->ring_buffer.count() > 0 )
		{
			md_old.set_album("");
			md_old.is_disabled = true;
			md_old.set_filepath("");

			QDateTime date = QDateTime::currentDateTime();
			QTime time = date.time();
			md_old.length_ms = (time.hour() * 60 + time.minute()) * 1000;

			emit sig_www_track_finished(md_old);
		}

		m->ring_buffer.insert(str);
	}

	emit sig_md_changed(md);
}

void PlayManager::shutdown()
{
	if(m->playstate == PlayState::Stopped){
		_settings->set<Set::PL_LastTrack>(-1);
		_settings->set<Set::Engine_CurTrackPos_s>(0);
	}

	else {
		_settings->set<Set::Engine_CurTrackPos_s>((int) (m->position_ms / 1000));
	}
}
