/* EngineHandler.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
	QList<RawSoundReceiverInterface*>	rawSoundReceiver;
	QList<LevelReceiver*>				levelReceiver;
	QList<SpectrumReceiver*>			spectrumReceiver;

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

		spLog(Log::Error, this) << e.what();
		Message::error(QString(e.what()), "Engine");
		return;
	}

	auto* playManager = PlayManager::instance();
	connect(playManager, &PlayManager::sigPlaystateChanged,
			this, &Handler::playstateChanged);

	connect(playManager, &PlayManager::sigCurrentTrackChanged,
			this, [=](const MetaData& md){
				m->engine->changeTrack(md);
			});

	connect(playManager, &PlayManager::sigSeekedAbsoluteMs,
			m->engine, &Engine::jumpAbsMs);

	connect(playManager, &PlayManager::sigSeekedRelative,
			m->engine, &Engine::jumpRel);

	connect(playManager, &PlayManager::sigSeekedRelativeMs,
			m->engine, &Engine::jumpRelMs);

	connect(playManager, &PlayManager::sigRecording,
			m->engine, &Engine::setStreamRecorderRecording);

	const MetaData& md = playManager->currentTrack();
	if(!md.filepath().isEmpty()) {
		m->engine->changeTrack(md);
	}

	connect(m->engine, &Engine::sigDataAvailable, this, &Handler::newAudioDataAvailable);
	connect(m->engine, &Engine::sigCoverDataAvailable, this, &Handler::sigCoverDataAvailable);
	connect(m->engine, &Engine::sigError, playManager, &PlayManager::error);
	connect(m->engine, &Engine::sigCurrentPositionChanged, playManager, &PlayManager::setCurrentPositionMs);
	connect(m->engine, &Engine::sigTrackFinished, playManager, &PlayManager::setTrackFinished);
	connect(m->engine, &Engine::sigTrackReady, playManager, &PlayManager::setTrackReady);
	connect(m->engine, &Engine::sigBuffering, playManager, &PlayManager::buffering);

	connect(m->engine, &Engine::sigDurationChanged, this, [playManager](const MetaData& md){
		playManager->changeDuration(md.durationMs());
	});

	connect(m->engine, &Engine::sigBitrateChanged, this, [playManager](const MetaData& md){
		playManager->changeBitrate(md.bitrate());
	});

	connect(m->engine, &Engine::sigMetadataChanged, this, [playManager](const MetaData& md){
		playManager->changeCurrentMetadata(md);
	});

	connect(m->engine, &Engine::sigSpectrumChanged, this, &Handler::spectrumChanged);
	connect(m->engine, &Engine::sigLevelChanged, this, &Handler::levelChanged);
}

Handler::~Handler() {}

void Handler::shutdown()
{
	if(m->engine){
		delete m->engine; m->engine=nullptr;
	}
}

bool Handler::isValid() const
{
	return (m->engine != nullptr);
}

void Handler::playstateChanged(PlayState state)
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


void Handler::newAudioDataAvailable(const QByteArray& data)
{
	for(auto* receiver : Algorithm::AsConst(m->rawSoundReceiver))
	{
		receiver->writeAudioData(data);
	}
}

void Handler::spectrumChanged()
{
	for(SpectrumReceiver* rcv : m->spectrumReceiver)
	{
		if(rcv && rcv->isActive())
		{
			SpectrumList vals = m->engine->spectrum();
			rcv->setSpectrum(vals);
		}
	}
}

void Handler::levelChanged()
{
	for(LevelReceiver* rcv : m->levelReceiver)
	{
		if(rcv && rcv->isActive())
		{
			QPair<float, float> level = m->engine->level();
			rcv->setLevel(level.first, level.second);
		}
	}
}

void Handler::reloadReceivers()
{
	bool s = Util::Algorithm::contains(m->spectrumReceiver, [](SpectrumReceiver* spectrumReceiver){
		return (spectrumReceiver->isActive());
	});

	bool l = Util::Algorithm::contains(m->levelReceiver, [](LevelReceiver* levelReceiver){
		return (levelReceiver->isActive());
	});

	m->engine->setVisualizerEnabled(l, s);
}

void Handler::registerRawSoundReceiver(RawSoundReceiverInterface* receiver)
{
	if(!m->engine){
		return;
	}

	if(m->rawSoundReceiver.contains(receiver)){
		return;
	}

	m->rawSoundReceiver << receiver;
	m->engine->setBroadcastEnabled(!m->rawSoundReceiver.isEmpty());
}

void Handler::unregisterRawSoundReceiver(RawSoundReceiverInterface* receiver)
{
	if(!m->engine){
		return;
	}

	if(!m->rawSoundReceiver.contains(receiver)){
		return;
	}

	m->rawSoundReceiver.removeOne(receiver);
	m->engine->setBroadcastEnabled(!m->rawSoundReceiver.isEmpty());
}

void Handler::registerLevelReceiver(LevelReceiver* receiver)
{
	if(!m->levelReceiver.contains(receiver)) {
		m->levelReceiver.push_back(receiver);
	}

	reloadReceivers();
}

void Handler::reloadLevelReceivers()
{
	reloadReceivers();
}

void Handler::registerSpectrumReceiver(SpectrumReceiver* receiver)
{
	if(!m->spectrumReceiver.contains(receiver)) {
		m->spectrumReceiver.push_back(receiver);
	}

	reloadReceivers();
}

void Handler::reloadSpectrumReceivers()
{
	reloadReceivers();
}

void Handler::setEqualizer(int band, int value)
{
	if(!m->engine){
		return;
	}

	m->engine->setEqualizer(band, value);
}
