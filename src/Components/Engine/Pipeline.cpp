/* PlaybackPipeline.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "Pipeline.h"

#include "Components/Engine/EngineUtils.h"
#include "Components/Engine/Callbacks.h"

#include "PipelineExtensions/Probing.h"
#include "PipelineExtensions/VisualizerBin.h"
#include "PipelineExtensions/BroadcastBin.h"
#include "StreamRecorder/StreamRecorderBin.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Logger/Logger.h"

#include <QTimer>

#include <gst/app/gstappsink.h>

#include <algorithm>
#include <cmath>

using Engine::Pipeline;
using namespace PipelineExtensions;
namespace EngineUtils=::Engine::Utils;
namespace Callbacks=::Engine::Callbacks;

struct Pipeline::Private
{
	QString				name;

	GstElement*			pipeline=nullptr;
	GstElement*			source=nullptr;
	GstElement*			audioConvert=nullptr;
	GstElement*			pitch=nullptr;
	GstElement*			equalizer=nullptr;
	GstElement*			tee=nullptr;

	GstElement*			positionElement=nullptr;

	GstElement*			playbackBin=nullptr;
	GstElement*			playbackQueue=nullptr;
	GstElement*			playbackVolume=nullptr;
	GstElement*			playbackSink=nullptr;

	StreamRecorderBin*	streamRecorder=nullptr;
	BroadcastBin*		broadcaster=nullptr;
	VisualizerBin*		visualizer=nullptr;

	QTimer*				progressTimer=nullptr;

	Private(const QString& name) :
		name(name)
	{}

	GstElement* createSink(const QString& sinkName)
	{
		static int Number=1;

		GstElement* ret=nullptr;

		QString name = sinkName + QString::number(Number);

		if(sinkName == "pulse")
		{
			spLog(Log::Debug, this) << "Create pulseaudio sink";
			EngineUtils::createElement(&ret, "pulsesink", name.toLocal8Bit().data());
		}

		else if(sinkName == "alsa")
		{
			spLog(Log::Debug, this) << "Create alsa sink";
			QString device = GetSetting(Set::Engine_AlsaDevice);
			EngineUtils::createElement(&ret, "alsasink", name.toLocal8Bit().data());

			if(device.isEmpty()) {
				device = "default";
			}

			spLog(Log::Info, this) << "Created alsa sink with " << device << " as output";
		}

		if(ret == nullptr)
		{
			spLog(Log::Debug, this) << "Will create auto audio sink";
			EngineUtils::createElement(&ret, "autoaudiosink", name.toLocal8Bit().data());
		}

		return ret;
	}
};

Pipeline::Pipeline(const QString& name, QObject* parent) :
	QObject(parent),
	PipelineExtensions::Fadeable(),
	PipelineExtensions::Changeable(),
	PipelineExtensions::DelayedPlayable(),
	PipelineExtensions::BroadcastDataReceiver(),
	PipelineExtensions::PositionAccessible(),
	PipelineExtensions::Pitchable(),
	PipelineExtensions::EqualizerAccessible()
{
	m = Pimpl::make<Private>(name);
}

Pipeline::~Pipeline()
{
	if(m->pipeline)
	{
		EngineUtils::setState(m->pipeline, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT(m->pipeline));
		m->pipeline = nullptr;
	}
}

bool Pipeline::init(Engine* engine)
{
	if(m->pipeline) {
		return true;
	}

	m->pipeline = gst_pipeline_new(m->name.toStdString().c_str());
	m->positionElement = m->pipeline;

	if(!EngineUtils::testAndError(m->pipeline, "Engine: Pipeline sucks")){
		return false;
	}

	bool success = createElements();
	if(!success) {
		return false;
	}

	success = addAndLinkElements();
	if(!success) {
		return false;
	}

	configureElements();
	EngineUtils::setState(m->pipeline, GST_STATE_NULL);

	GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(m->pipeline));
#ifdef Q_OS_WIN
	gst_bus_set_sync_handler(bus, Engine::Callbacks::bus_message_received, engine, EngineCallbacks::destroy_notify);
#else
	gst_bus_add_watch(bus, Callbacks::busStateChanged, engine);
#endif

	m->progressTimer = new QTimer(this);
	m->progressTimer->setTimerType(Qt::PreciseTimer);
	m->progressTimer->setInterval( EngineUtils::getUpdateInterval() );
	connect(m->progressTimer, &QTimer::timeout, this, [=]()
	{
		if(EngineUtils::getState(m->pipeline) != GST_STATE_NULL){
			Callbacks::positionChanged(this);
		}
	});

	m->progressTimer->start();

	SetSetting(SetNoDB::MP3enc_found, EngineUtils::isLameAvailable());
	SetSetting(SetNoDB::Pitch_found, EngineUtils::isPitchAvailable());

	ListenSetting(Set::Engine_Vol, Pipeline::volumeChanged);
	ListenSetting(Set::Engine_Mute, Pipeline::muteChanged);
	ListenSettingNoCall(Set::Engine_Sink, Pipeline::sinkChanged);

	// set by gui, initialized directly in pipeline
	ListenSettingNoCall(Set::Engine_Pitch, Pipeline::sppedChanged);
	ListenSettingNoCall(Set::Engine_Speed, Pipeline::sppedChanged);
	ListenSettingNoCall(Set::Engine_PreservePitch, Pipeline::sppedChanged);
	ListenSetting(Set::Engine_SpeedActive, Pipeline::speedActiveChanged);

	return true;
}

bool Pipeline::createElements()
{
	// input
	if(!EngineUtils::createElement(&m->source, "uridecodebin", "src")) return false;
	if(!EngineUtils::createElement(&m->audioConvert, "audioconvert")) return false;
	if(!EngineUtils::createElement(&m->tee, "tee")) return false;
	if(!EngineUtils::createElement(&m->playbackQueue, "queue", "playback_queue")) return false;
	if(!EngineUtils::createElement(&m->playbackVolume, "volume")) return false;

	// optional pitch
	EngineUtils::createElement(&m->pitch, "pitch");

	EngineUtils::createElement(&m->equalizer, "equalizer-10bands");

	m->playbackSink = m->createSink(GetSetting(Set::Engine_Sink));

	m->visualizer = new VisualizerBin(m->pipeline, m->tee);
	m->broadcaster = new BroadcastBin(this, m->pipeline, m->tee);

	return (m->playbackSink != nullptr);
}


bool Pipeline::addAndLinkElements()
{
	{ // before tee
		EngineUtils::addElements
		(
			GST_BIN(m->pipeline),
			{m->source, m->audioConvert, m->equalizer, m->tee}
		);

		bool success = EngineUtils::linkElements({
			m->audioConvert, m->equalizer, m->tee
		});

		if(!EngineUtils::testAndErrorBool(success, "Engine: Cannot link audio convert with tee")){
			return false;
		}
	}

	{ // Playback Bin
		m->playbackBin = gst_bin_new("Playback_bin");
		EngineUtils::addElements(GST_BIN(m->playbackBin), {m->playbackQueue, m->playbackVolume, m->playbackSink});

		bool success = EngineUtils::linkElements({m->playbackQueue, m->playbackVolume, m->playbackSink});
		if(!EngineUtils::testAndErrorBool(success, "Engine: Cannot link eq with audio sink")) {
			return false;
		}

		EngineUtils::addElements(GST_BIN(m->pipeline), {m->playbackBin});

		EngineUtils::createGhostPad(GST_BIN(m->playbackBin), m->playbackQueue);
		EngineUtils::connectTee(m->tee, m->playbackBin, "Equalizer");

		return EngineUtils::testAndErrorBool(success, "Engine: Cannot link eq queue with tee");
	}
}

void Pipeline::configureElements()
{
	EngineUtils::setValues(G_OBJECT(m->tee),
				"silent", true,
				"allow-not-linked", true);

	EngineUtils::configureQueue(m->playbackQueue);

	g_signal_connect (m->source, "pad-added", G_CALLBACK (Callbacks::decodebinReady), m->audioConvert);
	g_signal_connect (m->source, "source-setup", G_CALLBACK (Callbacks::sourceReady), nullptr);

	m->positionElement = m->source;
}


bool Pipeline::prepare(const QString& uri)
{
	stop();
	EngineUtils::setValues(G_OBJECT(m->source),
		"use-buffering", Util::File::isWWW(uri),
		"uri", uri.toUtf8().data()
	);

	EngineUtils::setInt64Value(G_OBJECT(m->source), "buffer-duration", GetSetting(Set::Engine_BufferSizeMS));
	EngineUtils::setState(m->pipeline, GST_STATE_PAUSED);

	volumeChanged();
	muteChanged();

	return true;
}

void Pipeline::play()
{
	EngineUtils::setState(m->pipeline, GST_STATE_PLAYING);
}

void Pipeline::pause()
{
	EngineUtils::setState(m->pipeline, GST_STATE_PAUSED);
}

void Pipeline::stop()
{
	EngineUtils::setState(m->pipeline, GST_STATE_NULL);

	abortDelayedPlaying();
	abortFader();
}

void Pipeline::volumeChanged()
{
	double vol = GetSetting(Set::Engine_Vol) / 100.0;
	setInternalVolume(vol);
}

void Pipeline::muteChanged()
{
	bool muted = GetSetting(Set::Engine_Mute);
	EngineUtils::setValue(G_OBJECT(m->playbackVolume), "mute", muted);
}

void Pipeline::setVisualizerEnabled(bool levelEnabled, bool spectrumEnabled)
{
	m->visualizer->setEnabled(levelEnabled, spectrumEnabled);
}

bool Pipeline::isLevelVisualizerEnabled() const
{
	return m->visualizer->isLevelEnabled();
}

bool Pipeline::isSpectrumVisualizerEnabled() const
{
	return m->visualizer->isSpectrumEnabled();
}

void Pipeline::setBroadcastingEnabled(bool b)
{
	m->broadcaster->setEnabled(b);
}

bool Pipeline::isBroadcastingEnabled() const
{
	return m->broadcaster->isEnabled();
}

GstState Pipeline::state() const
{
	return EngineUtils::getState(m->pipeline);
}

MilliSeconds Pipeline::timeToGo() const
{
	GstElement* element = m->pipeline;
	MilliSeconds ms = EngineUtils::getTimeToGo(element);

	return std::max<MilliSeconds>(ms - 100, 0);
}

void Pipeline::setRawData(const QByteArray& data)
{
	emit sigDataAvailable(data);
}

void Pipeline::record(bool b)
{
	if(!m->streamRecorder)
	{
		m->streamRecorder = new StreamRecorderBin(m->pipeline, m->tee);
		m->streamRecorder->init();
	}

	m->streamRecorder->setEnabled(b);
}

void Pipeline::setRecordingPath(const QString& path)
{
	m->streamRecorder->setTargetPath(path);
}

void Pipeline::setInternalVolume(double volume)
{
	EngineUtils::setValue(G_OBJECT(m->playbackVolume), "volume", volume);
}

double Pipeline::internalVolume() const
{
	double volume;
	g_object_get(m->playbackVolume, "volume", &volume, nullptr);
	return volume;
}


void Pipeline::speedActiveChanged()
{
	if(!m->pitch) {
		return;
	}

	if(GetSetting(Set::Engine_SpeedActive))
	{
		Changeable::addElement(m->pitch, m->audioConvert, m->equalizer);
		sppedChanged();
	}

	else
	{
		Changeable::removeElement(m->pitch, m->audioConvert, m->equalizer);
	}

	if(EngineUtils::getState(m->pipeline) == GST_STATE_PLAYING)
	{
		MilliSeconds positionMs = std::max<MilliSeconds>(this->positionMs(), 0);
		this->seekNearestMs(positionMs);
	}

	checkPosition();
}

void Pipeline::sppedChanged()
{
	Pitchable::setSpeed
	(
		GetSetting(Set::Engine_Speed),
		GetSetting(Set::Engine_Pitch) / 440.0,
		GetSetting(Set::Engine_PreservePitch)
	);
}

void Pipeline::sinkChanged()
{
	GstElement* newSink = m->createSink(GetSetting(Set::Engine_Sink));
	if(!newSink){
		return;
	}

	replaceSink(m->playbackSink, newSink, m->playbackVolume, m->pipeline, m->playbackBin);
	m->playbackSink = newSink;
}

void Pipeline::checkPosition()
{
	MilliSeconds positionMs = this->positionMs();
	positionMs = std::max<MilliSeconds>(0, positionMs);

	emit sigPositionChangedMs( positionMs );

	checkAboutToFinish();
}

void Pipeline::checkAboutToFinish()
{
	MilliSeconds positionMs = this->positionMs();
	MilliSeconds durationMs = this->durationMs();
	MilliSeconds aboutToFinishMs = std::max<MilliSeconds>(fadingTimeMs(), 300);

	static bool aboutToFinish = false;

	if(durationMs < positionMs || (durationMs <= aboutToFinishMs ) || (positionMs <= 0))
	{
		aboutToFinish = false;
		return;
	}

	MilliSeconds difference = durationMs - positionMs;

	aboutToFinish = (difference < aboutToFinishMs);

	if(aboutToFinish)
	{
		spLog(Log::Develop, this) << "About to finish in " << difference << ": Dur: " << durationMs << ", Pos: " << positionMs;
		emit sigAboutToFinishMs(difference);
	}
}

bool Pipeline::hasElement(GstElement* e) const
{
	return EngineUtils::hasElement(GST_BIN(m->pipeline), e);
}

GstElement* Pipeline::positionElement() const
{
	return m->positionElement;
}

GstElement* Pipeline::pitchElement() const
{
	return m->pitch;
}

GstElement* Pipeline::equalizerElement() const
{
	return m->equalizer;
}

void Pipeline::postProcessFadeIn() {}

void Pipeline::postProcessFadeOut() {}
