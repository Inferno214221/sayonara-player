/* EngineHandler.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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
#include "Engine.h"

#include "Components/PlayManager/PlayManager.h"
#include "Interfaces/Engine/AudioDataReceiverInterface.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Message/Message.h"

using Engine::Handler;
namespace Algorithm=Util::Algorithm;

struct Handler::Private
{
	QList<RawSoundReceiverInterface*>	raw_sound_receiver;
	QList<LevelReceiver*>		level_receiver;
	QList<SpectrumReceiver*>	spectrum_receiver;

	Engine* engine=nullptr;
};

Handler::Handler(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	try
	{
		m->engine = new Engine(this);
	}

	catch (std::exception& e)
	{
		m->engine = nullptr;

		sp_log(Log::Error, this) << e.what();
		Message::error(QString(e.what()), "Engine");
		return;
	}

	auto* play_manager = PlayManager::instance();
	connect(play_manager, &PlayManager::sig_playstate_changed,
			this, &Handler::playstate_changed);

	connect(play_manager, &PlayManager::sig_track_changed,
			this, [=](const MetaData& md){
				m->engine->change_track(md);
			});

	connect(play_manager, &PlayManager::sig_seeked_abs_ms,
			m->engine, &Engine::jump_abs_ms);

	connect(play_manager, &PlayManager::sig_seeked_rel,
			m->engine, &Engine::jump_rel);

	connect(play_manager, &PlayManager::sig_seeked_rel_ms,
			m->engine, &Engine::jump_rel_ms);

	connect(play_manager, &PlayManager::sig_record,
			m->engine, &Engine::set_streamrecorder_recording);

	const MetaData& md = play_manager->current_track();
	if(!md.filepath().isEmpty()) {
		m->engine->change_track(md);
	}

	connect(m->engine, &Engine::sig_data, this, &Handler::new_data);
	connect(m->engine, &Engine::sig_cover_data, this, &Handler::sig_cover_data);
	connect(m->engine, &Engine::sig_error, play_manager, &PlayManager::error);
	connect(m->engine, &Engine::sig_current_position_changed, play_manager, &PlayManager::set_position_ms);
	connect(m->engine, &Engine::sig_track_finished, play_manager, &PlayManager::set_track_finished);
	connect(m->engine, &Engine::sig_track_ready, play_manager, &PlayManager::set_track_ready);
	connect(m->engine, &Engine::sig_buffering, play_manager, &PlayManager::buffering);

	connect(m->engine, &Engine::sig_duration_changed, this, [play_manager](const MetaData& md){
		play_manager->change_duration(md.duration_ms());
	});

	connect(m->engine, &Engine::sig_bitrate_changed, this, [play_manager](const MetaData& md){
		play_manager->change_bitrate(md.bitrate());
	});

	connect(m->engine, &Engine::sig_metadata_changed, this, [play_manager](const MetaData& md){
		play_manager->change_track_metadata(md);
	});

	connect(m->engine, &Engine::sig_spectrum_changed, this, &Handler::spectrum_changed);
	connect(m->engine, &Engine::sig_level_changed, this, &Handler::level_changed);
}

Handler::~Handler() = default;

void Handler::shutdown()
{
	if(m->engine){
		delete m->engine; m->engine=nullptr;
	}
}

bool Handler::is_valid() const
{
	return (m->engine != nullptr);
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


void Handler::new_data(const QByteArray& data)
{
	for(auto* receiver : Algorithm::AsConst(m->raw_sound_receiver))
	{
		receiver->new_audio_data(data);
	}
}

void Handler::spectrum_changed()
{
	for(SpectrumReceiver* rcv : m->spectrum_receiver)
	{
		if(rcv && rcv->is_active())
		{
			SpectrumList vals = m->engine->spectrum();
			rcv->set_spectrum(vals);
		}
	}
}

void Handler::level_changed()
{
	for(LevelReceiver* rcv : m->level_receiver)
	{
		if(rcv && rcv->is_active())
		{
			QPair<float, float> level = m->engine->level();
			rcv->set_level(level.first, level.second);
		}
	}
}

void Handler::register_raw_sound_receiver(RawSoundReceiverInterface* receiver)
{
	if(!m->engine){
		return;
	}

	if(m->raw_sound_receiver.contains(receiver)){
		return;
	}

	m->raw_sound_receiver << receiver;
	m->engine->set_broadcast_enabled(!m->raw_sound_receiver.isEmpty());
}

void Handler::unregister_raw_sound_receiver(RawSoundReceiverInterface* receiver)
{
	if(!m->engine){
		return;
	}

	if(!m->raw_sound_receiver.contains(receiver)){
		return;
	}

	m->raw_sound_receiver.removeOne(receiver);
	m->engine->set_broadcast_enabled(!m->raw_sound_receiver.isEmpty());
}

void Handler::add_level_receiver(LevelReceiver* receiver)
{
	if(!m->level_receiver.contains(receiver)) {
		m->level_receiver.push_back(receiver);
	}
}

void Handler::add_spectrum_receiver(SpectrumReceiver* receiver)
{
	if(!m->spectrum_receiver.contains(receiver)) {
		m->spectrum_receiver.push_back(receiver);
	}
}

void Handler::set_equalizer(int band, int value)
{
	if(!m->engine){
		return;
	}

	m->engine->set_equalizer(band, value);
}
