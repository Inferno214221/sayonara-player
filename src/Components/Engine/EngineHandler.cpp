/* EngineHandler.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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
#include "Components/Engine/Playback/SoundOutReceiver.h"
#include "Interfaces/RawSoundReceiver/RawSoundReceiverInterface.h"

#include "Playback/PlaybackEngine.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"

#include <QImage>

using Engine::Handler;
using Engine::Base;
using Engine::Playback;

struct Handler::Private
{
	PlayManagerPtr						play_manager=nullptr;
	QList<RawSoundReceiverInterface*>	raw_sound_receiver;

	Engine::Playback*					engine=nullptr;

	Private()
	{
		play_manager = PlayManager::instance();
	}
};

Handler::Handler(QObject* parent) :
	Base(Name::EngineHandler, parent)
{
	m = Pimpl::make<Private>();
	m->engine = new Engine::Playback(this);
	m->engine->init();

	configure_connections();

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
	delete m->engine;
}


void Handler::playstate_changed(PlayState state)
{
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
	m->engine->play();
}

void Handler::stop()
{
	m->engine->stop();
}


void Handler::pause()
{
	m->engine->pause();
}


void Handler::jump_abs_ms(MilliSeconds ms)
{
	m->engine->jump_abs_ms(ms);
}

void Handler::jump_rel_ms(MilliSeconds ms)
{
	m->engine->jump_rel_ms(ms);
}

void Handler::jump_rel(double where)
{
	m->engine->jump_rel(where);
}


bool Handler::change_track(const MetaData& md)
{
	return m->engine->change_track(md);
}

bool Handler::change_track_by_filename(const QString& filepath)
{
	return m->engine->change_track_by_filename(filepath);
}


void Handler::sl_md_changed(const MetaData& md)
{
	m->play_manager->change_metadata(md);
	emit sig_md_changed(md);
}

void Handler::sl_dur_changed(const MetaData& md)
{
	m->play_manager->change_duration(md.length_ms);
	emit sig_duration_changed(md);
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
	m->engine->set_streamrecorder_recording(b);
}

bool Handler::configure_connections()
{
	connect(m->engine, &Base::sig_track_ready, this, &Handler::sl_track_ready_changed);
	connect(m->engine, &Base::sig_md_changed, this, &Handler::sl_md_changed);
	connect(m->engine, &Base::sig_pos_changed_ms, this, &Handler::sl_pos_changed_ms);
	connect(m->engine, &Base::sig_duration_changed, this, &Handler::sl_dur_changed);
	connect(m->engine, &Base::sig_bitrate_changed, this, &Handler::sig_bitrate_changed);
	connect(m->engine, &Base::sig_track_finished, this, &Handler::sl_track_finished);
	connect(m->engine, &Base::sig_buffering, this, &Handler::sl_buffer_state_changed);
	connect(m->engine, &Base::sig_cover_changed, this, &Handler::sig_cover_changed);
	connect(m->engine, &Base::sig_error, this, &Handler::sl_error);

	return true;
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
	m->engine->set_n_sound_receiver(m->raw_sound_receiver.size());
}

void Handler::unregister_raw_sound_receiver(RawSoundReceiverInterface* receiver)
{
	if(!m->raw_sound_receiver.contains(receiver)){
		return;
	}

	m->raw_sound_receiver.removeOne(receiver);
	m->engine->set_n_sound_receiver(m->raw_sound_receiver.size());
}

void Handler::register_level_receiver(LevelReceiver* receiver)
{
	m->engine->add_level_receiver(receiver);
}

void Handler::register_spectrum_receiver(SpectrumReceiver* receiver)
{
	m->engine->add_spectrum_receiver(receiver);
}


void Handler::set_equalizer(int band, int value)
{
	m->engine->set_equalizer(band, value);
}


bool Handler::change_uri(const QString& uri)
{
	Q_UNUSED(uri);
	return true;
}
