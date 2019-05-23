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

using Engine::Handler;
using Engine::Playback;

struct Handler::Private
{
	QList<RawSoundReceiverInterface*>	raw_sound_receiver;
	Engine::Playback*					engine=nullptr;
};

Handler::Handler(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	m->engine = new Engine::Playback(this);
	m->engine->init();

	PlayManager* play_manager = PlayManager::instance();

	connect(play_manager, &PlayManager::sig_playstate_changed,
			this, &Handler::playstate_changed);

	connect(play_manager, &PlayManager::sig_track_changed,
			this, [=](const MetaData& md){
				m->engine->change_track(md);
			});

	connect(play_manager, &PlayManager::sig_seeked_abs_ms,
			m->engine, &Playback::jump_abs_ms);

	connect(play_manager, &PlayManager::sig_seeked_rel,
			m->engine, &Playback::jump_rel);

	connect(play_manager, &PlayManager::sig_seeked_rel_ms,
			m->engine, &Playback::jump_rel_ms);

	connect(play_manager, &PlayManager::sig_record,
			m->engine, &Playback::set_streamrecorder_recording);

	const MetaData& md = play_manager->current_track();
	if(!md.filepath().isEmpty()) {
		m->engine->change_track(md);
	}

	connect(m->engine, &Playback::sig_md_changed, this, &Handler::sig_md_changed);
	connect(m->engine, &Playback::sig_duration_changed, this, &Handler::sig_duration_changed);
	connect(m->engine, &Playback::sig_bitrate_changed, this, &Handler::sig_bitrate_changed);
	connect(m->engine, &Playback::sig_cover_changed, this, &Handler::sig_cover_changed);
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
	switch(state)
	{
		case PlayState::Playing:
			m->engine->play();
			break;

		case PlayState::Paused:
			m->engine->pause();
			break;

		case PlayState::Stopped:
			m->engine->stop();
			break;
		default:
			return;
	}
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

void Handler::add_level_receiver(LevelReceiver* receiver)
{
	m->engine->add_level_receiver(receiver);
}

void Handler::add_spectrum_receiver(SpectrumReceiver* receiver)
{
	m->engine->add_spectrum_receiver(receiver);
}

void Handler::set_equalizer(int band, int value)
{
	m->engine->set_equalizer(band, value);
}
