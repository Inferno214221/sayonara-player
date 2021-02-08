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

#include "Interfaces/CoverDataProvider.h"
#include "Interfaces/PlayManager.h"
#include "Interfaces/Engine/AudioDataReceiver.h"
#include "Interfaces/Engine/CoverDataReceiver.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Message/Message.h"

#include <QString>
#include <QByteArray>

#include <set>

using Engine::Handler;
namespace Algorithm = Util::Algorithm;

namespace
{
	template<typename T>
	void unregisterReceiver(T* receiver, std::set<T*>& receivers)
	{
		auto it = receivers.find(receiver);
		if(it != receivers.end())
		{
			receivers.erase(it);
		}
	}
}

struct Handler::Private
{
	std::set<RawAudioDataReceiver*> rawSoundReceiver;
	std::set<LevelDataReceiver*> levelReceivers;
	std::set<SpectrumDataReceiver*> spectrumReceivers;
	std::set<CoverDataReceiver*> coverReceivers;

	Engine* engine = nullptr;
};

Handler::Handler(QObject* parent) :
	QObject(parent),
	CoverDataProvider()
{
	m = Pimpl::make<Private>();
}

void Handler::init(PlayManager* playManager)
{
	try
	{
		m->engine = new Engine(playManager, this);
	}

	catch(std::exception& e)
	{
		m->engine = nullptr;

		spLog(Log::Error, this) << e.what();
		Message::error(QString(e.what()), "Engine");
		return;
	}

	connect(playManager, &PlayManager::sigPlaystateChanged,
	        this, &Handler::playstateChanged);

	connect(playManager, &PlayManager::sigCurrentTrackChanged,
	        this, [=](const MetaData& md) {
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
	if(!md.filepath().isEmpty())
	{
		m->engine->changeTrack(md);
	}

	connect(m->engine, &Engine::sigDataAvailable, this, &Handler::setAudioData);
	connect(m->engine, &Engine::sigCoverDataAvailable, this, &Handler::setCoverData);
	connect(m->engine, &Engine::sigError, playManager, &PlayManager::error);
	connect(m->engine, &Engine::sigCurrentPositionChanged, playManager, &PlayManager::setCurrentPositionMs);
	connect(m->engine, &Engine::sigTrackFinished, playManager, &PlayManager::setTrackFinished);
	connect(m->engine, &Engine::sigTrackReady, playManager, &PlayManager::setTrackReady);
	connect(m->engine, &Engine::sigBuffering, playManager, &PlayManager::buffering);

	connect(m->engine, &Engine::sigDurationChanged, this, [playManager](const MetaData& md) {
		playManager->changeDuration(md.durationMs());
	});

	connect(m->engine, &Engine::sigBitrateChanged, this, [playManager](const MetaData& md) {
		playManager->changeBitrate(md.bitrate());
	});

	connect(m->engine, &Engine::sigMetadataChanged, this, [playManager](const MetaData& md) {
		playManager->changeCurrentMetadata(md);
	});

	connect(m->engine, &Engine::sigSpectrumChanged, this, &Handler::spectrumChanged);
	connect(m->engine, &Engine::sigLevelChanged, this, &Handler::levelChanged);
}

Handler::~Handler() = default;

void Handler::shutdown()
{
	if(m->engine)
	{
		delete m->engine;
		m->engine = nullptr;
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



void Handler::registerSpectrumReceiver(SpectrumDataReceiver* receiver)
{
	m->spectrumReceivers.insert(receiver);
	reloadReceivers();
}

void Engine::Handler::unregisterSpectrumReceiver(SpectrumDataReceiver* spectrumReceiver)
{
	unregisterReceiver(spectrumReceiver, m->spectrumReceivers);
}

void Engine::Handler::setSpectrumData(const std::vector<float>& spectrum)
{
	for(auto* receiver : m->spectrumReceivers)
	{
		if(receiver->isActive())
		{
			receiver->setSpectrum(spectrum);
		}
	}
}

void Handler::spectrumChanged()
{
	setSpectrumData(m->engine->spectrum());
}

void Handler::spectrumActiveChanged(bool /*b*/)
{
	reloadReceivers();
}



void Handler::registerLevelReceiver(LevelDataReceiver* receiver)
{
	m->levelReceivers.insert(receiver);
	reloadReceivers();
}

void Engine::Handler::unregisterLevelReceiver(LevelDataReceiver* levelReceiver)
{
	unregisterReceiver(levelReceiver, m->levelReceivers);
}


void Engine::Handler::setLevelData(float left, float right)
{
	for(auto* receiver : m->levelReceivers)
	{
		if(receiver->isActive())
		{
			receiver->setLevel(left, right);
		}
	}
}

void Handler::levelChanged()
{
	QPair<float, float> level = m->engine->level();
	setLevelData(level.first, level.second);
}

void Handler::levelActiveChanged(bool /*b*/)
{
	reloadReceivers();
}



void Handler::reloadReceivers()
{
	bool s = Util::Algorithm::contains(m->spectrumReceivers, [](SpectrumDataReceiver* spectrumReceiver) {
		return (spectrumReceiver->isActive());
	});

	bool l = Util::Algorithm::contains(m->levelReceivers, [](LevelDataReceiver* levelReceiver) {
		return (levelReceiver->isActive());
	});

	m->engine->setVisualizerEnabled(l, s);
}


void Handler::setEqualizer(int band, int value)
{
	if(!m->engine)
	{
		return;
	}

	m->engine->setEqualizer(band, value);
}



void Engine::Handler::registerCoverReceiver(CoverDataReceiver* coverDataReceiver)
{
	m->coverReceivers.insert(coverDataReceiver);
}

void Engine::Handler::unregisterCoverReceiver(CoverDataReceiver* coverDataReceiver)
{
	unregisterReceiver(coverDataReceiver, m->coverReceivers);
}

void Engine::Handler::setCoverData(const QByteArray& imageData, const QString& mimeData)
{
	for(auto* receiver : m->coverReceivers)
	{
		if(receiver->isActive())
		{
			receiver->setCoverData(imageData, mimeData);
		}
	}
}

void Engine::Handler::registerAudioDataReceiver(RawAudioDataReceiver* receiver)
{
	m->rawSoundReceiver.insert(receiver);
	m->engine->setBroadcastEnabled(!m->rawSoundReceiver.empty());
}

void Engine::Handler::unregisterAudioDataReceiver(RawAudioDataReceiver* receiver)
{
	unregisterReceiver(receiver, m->rawSoundReceiver);
	m->engine->setBroadcastEnabled(!m->rawSoundReceiver.empty());
}

void Engine::Handler::setAudioData(const QByteArray& data)
{
	for(auto* receiver : Algorithm::AsConst(m->rawSoundReceiver))
	{
		receiver->writeAudioData(data);
	}
}
