/* EngineHandler.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EngineHandler.h"

#include "Components/PlayManager/PlayManager.h"
#include "Interfaces/RawSoundReceiver/RawSoundReceiverInterface.h"

#include "Playback/PlaybackEngine.h"
#include "Convert/ConvertEngine.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"

#include <QImage>

using Engine::Handler;
using Engine::Base;
using Engine::Playback;
using Engine::Convert;

struct Handler::Private
{
	PlayManagerPtr						play_manager=nullptr;
	Engine::Base*                       cur_engine=nullptr;
	QList<Base*>						engines;
	QList<RawSoundReceiverInterface*>	raw_sound_receiver;

	Private()
	{
		play_manager = PlayManager::instance();
	}
};

Handler::Handler(QObject* parent) :
	Base(Name::EngineHandler, parent)
{
	m = Pimpl::make<Private>();

	connect(m->play_manager, &PlayManager::sig_playstate_changed,
			this, &Handler::playstate_changed);

	connect(m->play_manager, &PlayManager::sig_track_changed,
			this, [=](const MetaData& md){
				this->change_track(md);
			});

	connect(m->play_manager, &PlayManager::sig_seeked_abs_ms,
			this, &Handler::jump_abs_ms);

	connect(m->play_manager, &PlayManager::sig_seeked_rel,
			this, &Handler::jump_rel);

	connect(m->play_manager, &PlayManager::sig_seeked_rel_ms,
			this, &Handler::jump_rel_ms);

	connect(m->play_manager, &PlayManager::sig_record,
			this, &Handler::sr_record_button_pressed);

	switch_engine(Name::PlaybackEngine);

	const MetaData& md = m->play_manager->current_track();
	if(!md.filepath().isEmpty()) {
		change_track(md);
	}
}

Handler::~Handler() {}

bool Handler::init()
{
	return true;
}

void Handler::shutdown()
{
	for(Base* e : ::Util::AsConst(m->engines))
	{
		e->stop();
		delete e;
	}

	m->engines.clear();
}


void Handler::start_convert()
{
	stop();

	if( m->cur_engine->name() != Name::ConvertEngine ) {
		switch_engine(Name::ConvertEngine);
	}

	m->cur_engine->stop();
}

void Handler::end_convert()
{
	stop();

	sp_log(Log::Debug, this) << "Engine end convert";

	if( m->cur_engine->name() != Name::PlaybackEngine ) {
		sp_log(Log::Debug, this) << "Change to playback engine";
		switch_engine(Name::PlaybackEngine);
	}

	m->cur_engine->stop();
}


void Handler::playstate_changed(PlayState state)
{
	if(!m->cur_engine) return;

	switch(state){
		case PlayState::Playing:
			play();
			break;

		case PlayState::Paused:
			pause();
			break;

		case PlayState::Stopped:
			stop();
			break;

		default:
			return;
	}
}


void Handler::play()
{
	if(!m->cur_engine) return;

	m->cur_engine->play();
}

void Handler::stop()
{
	if(!m->cur_engine) return;
	m->cur_engine->stop();
}

void Handler::pause()
{
	if(!m->cur_engine) return;
	m->cur_engine->pause();
}


void Handler::jump_abs_ms(MilliSeconds ms)
{
	if(!m->cur_engine) return;
	m->cur_engine->jump_abs_ms(ms);
}

void Handler::jump_rel_ms(MilliSeconds ms)
{
	if(!m->cur_engine) return;
	m->cur_engine->jump_rel_ms(ms);
}

void Handler::jump_rel(double where)
{
	if(!m->cur_engine) return;
	m->cur_engine->jump_rel(where);
}


bool Handler::change_track(const MetaData& md)
{
	if(!m->cur_engine) return false;
	return m->cur_engine->change_track(md);
}

bool Handler::change_track_by_filename(const QString& filepath)
{
	if(!m->cur_engine) return false;
	return m->cur_engine->change_track_by_filename(filepath);
}


void Handler::sl_md_changed(const MetaData& md)
{
	m->play_manager->change_metadata(md);
	emit sig_md_changed(md);
}

void Handler::sl_dur_changed(const MetaData& md)
{
	m->play_manager->change_duration(md.length_ms);
	emit sig_dur_changed(md);
}

void Handler::sl_pos_changed_ms(MilliSeconds ms)
{
	m->play_manager->set_position_ms(ms);
}

void Handler::sl_pos_changed_s(Seconds sec)
{
	m->play_manager->set_position_ms( (MilliSeconds) (sec * 1000) );
}

void Handler::sl_track_ready_changed()
{
	m->play_manager->set_track_ready();
}

void Handler::sl_track_finished()
{
	m->play_manager->next();
}

void Handler::sl_buffer_state_changed(int progress)
{
	m->play_manager->buffering(progress);
}

void Handler::sl_error(const QString& error_msg)
{
	m->play_manager->error(error_msg);
}

void Handler::sr_record_button_pressed(bool b)
{
	Playback* p = get_playback_engine();
	if(p){
		p->set_streamrecorder_recording(b);
	}
}

bool Handler::configure_connections(Base* old_engine, Base* new_engine)
{
	if(!old_engine && !new_engine) return false;
	if(old_engine == new_engine) return false;

	if(old_engine) {
		disconnect(old_engine, &Base::sig_track_ready, this, &Handler::sl_track_ready_changed);
		disconnect(old_engine, &Base::sig_md_changed, this, &Handler::sl_md_changed);
		disconnect(old_engine, &Base::sig_pos_changed_ms, this, &Handler::sl_pos_changed_ms);
		disconnect(old_engine, &Base::sig_dur_changed, this, &Handler::sl_dur_changed);
		disconnect(old_engine, &Base::sig_br_changed, this, &Base::sig_br_changed);
		disconnect(old_engine, &Base::sig_track_finished, this, &Handler::sl_track_finished);
		disconnect(old_engine, &Base::sig_buffering, this, &Handler::sl_buffer_state_changed);
		disconnect(old_engine, &Base::sig_cover_changed, this, &Handler::sig_cover_changed);
		disconnect(old_engine, &Base::sig_error, this, &Handler::sl_error);
	}

	if(new_engine) {
		connect(new_engine, &Base::sig_track_ready, this, &Handler::sl_track_ready_changed);
		connect(new_engine, &Base::sig_md_changed, this, &Handler::sl_md_changed);
		connect(new_engine, &Base::sig_pos_changed_ms, this, &Handler::sl_pos_changed_ms);
		connect(new_engine, &Base::sig_dur_changed, this, &Handler::sl_dur_changed);
		connect(new_engine, &Base::sig_br_changed, this, &Base::sig_br_changed);
		connect(new_engine, &Base::sig_track_finished, this, &Handler::sl_track_finished);
		connect(new_engine, &Base::sig_buffering, this, &Handler::sl_buffer_state_changed);
		connect(new_engine, &Base::sig_cover_changed, this, &Handler::sig_cover_changed);
		connect(new_engine, &Base::sig_error, this, &Handler::sl_error);
	}

	return true;
}

Base* Handler::get_engine(Name name)
{
	for(Base* e : ::Util::AsConst(m->engines))
	{
		if(e && e->name() == name){
			return e;
		}
	}

	if(name == Name::PlaybackEngine)
	{
		Playback* pb_engine = new Playback();
		if(pb_engine->init())
		{
			m->engines << static_cast<Base*>(pb_engine);
			connect(pb_engine, &Playback::sig_data, this, &Handler::new_data);
			return pb_engine;
		}
	}

	else if(name == Name::ConvertEngine)
	{
		Convert* cvt_engine = new Convert();
		if(cvt_engine->init())
		{
			m->engines << static_cast<Base*>(cvt_engine);
			return cvt_engine;
		}
	}

	return nullptr;
}


void Handler::switch_engine(Name name)
{
	Base* new_engine=get_engine(name);

	if(!new_engine){
		return;
	}

	configure_connections(m->cur_engine, new_engine);
	m->cur_engine = new_engine;
}

Engine::Playback* Handler::get_playback_engine()
{
	return dynamic_cast<Playback*>(get_engine(Name::PlaybackEngine));
}

void Handler::new_data(const uchar* data, uint64_t n_bytes)
{
	for(RawSoundReceiverInterface* receiver : ::Util::AsConst(m->raw_sound_receiver))
	{
		receiver->new_audio_data(data, n_bytes);
	}
}

void Handler::register_raw_sound_receiver(RawSoundReceiverInterface* receiver)
{
	if(m->raw_sound_receiver.contains(receiver)){
		return;
	}

	m->raw_sound_receiver << receiver;

	Playback* engine = get_playback_engine();
	if(engine){
		get_playback_engine()->set_n_sound_receiver(m->raw_sound_receiver.size());
	}
}

void Handler::unregister_raw_sound_receiver(RawSoundReceiverInterface* receiver)
{
	if(!m->raw_sound_receiver.contains(receiver)){
		return;
	}

	m->raw_sound_receiver.removeOne(receiver);

	Playback* engine = get_playback_engine();
	if(engine){
		get_playback_engine()->set_n_sound_receiver(m->raw_sound_receiver.size());
	}
}


void Handler::set_equalizer(int band, int value)
{
	Playback* engine = get_playback_engine();

	if(engine){
		engine->set_equalizer(band, value);
	}
}


bool Handler::change_uri(const QString& uri)
{
	Q_UNUSED(uri);
	return true;
}
