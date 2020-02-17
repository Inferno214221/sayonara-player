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

#include "Components/Engine/Engine.h"
#include "Components/Engine/EngineUtils.h"
#include "Components/Engine/Callbacks.h"

#include "PipelineExtensions/Probing.h"
#include "PipelineExtensions/Pitcher.h"
#include "PipelineExtensions/Equalizer.h"
#include "PipelineExtensions/Seeker.h"
#include "PipelineExtensions/Broadcaster.h"
#include "PipelineExtensions/Visualizer.h"

#include "StreamRecorder/StreamRecorderHandler.h"

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
namespace EngineUtils=::Engine::Utils;

namespace Callbacks=::Engine::Callbacks;
using namespace PipelineExtensions;

struct Pipeline::Private
{
	QString				name;

	GstElement*			pipeline=nullptr;
	GstElement*			source=nullptr;
	GstElement*			audioConvert=nullptr;
	GstElement*			tee=nullptr;

	GstElement*			playbackBin=nullptr;
	GstElement*			playbackQueue=nullptr;
	GstElement*			playbackVolume=nullptr;
	GstElement*			playbackSink=nullptr;

	GstElement*			positionElement=nullptr;

	Pitcher*			pitcher=nullptr;
	Seeker*				seeker=nullptr;
	StreamRecorderHandler* streamRecorder=nullptr;
	Broadcaster*		broadcaster=nullptr;
	Visualizer*			visualizer=nullptr;
	Equalizer*			equalizer=nullptr;

	QTimer*				progressTimer=nullptr;

	Private(const QString& name) :
		name(name)
	{}
};

Pipeline::Pipeline(const QString& name, QObject* parent) :
	QObject(parent),
	Fadeable(),
	Changeable(),
	DelayedPlayable(),
	BroadcastDataReceiver()
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

	// create equalizer element
	m->pipeline = gst_pipeline_new(m->name.toStdString().c_str());
	m->positionElement = m->pipeline;

	if(!EngineUtils::testAndError(m->pipeline, "Engine: Pipeline sucks")){
		return false;
	}

	GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(m->pipeline));

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
	ListenSetting(Set::Engine_ShowLevel, Pipeline::showVisualizerChanged);
	ListenSettingNoCall(Set::Engine_ShowSpectrum, Pipeline::showVisualizerChanged);
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

	m->playbackSink = createSink(GetSetting(Set::Engine_Sink));

	m->seeker = new Seeker(m->source);
	m->equalizer = new Equalizer();
	m->pitcher = new Pitcher();
	m->visualizer = new Visualizer(m->pipeline, m->tee);
	m->broadcaster = new Broadcaster(this, m->pipeline, m->tee);

	return (m->playbackSink != nullptr);
}

GstElement* Pipeline::createSink(const QString& _name)
{
	static int Number=1;

	GstElement* ret=nullptr;

	QString name = _name + QString::number(Number);

	if(_name == "pulse")
	{
		spLog(Log::Debug, this) << "Create pulseaudio sink";

		EngineUtils::createElement(&ret, "pulsesink", name.toLocal8Bit().data());
	}

	else if(_name == "alsa")
	{
		spLog(Log::Debug, this) << "Create alsa sink";
		QString device = GetSetting(Set::Engine_AlsaDevice);
		EngineUtils::createElement(&ret, "alsasink", name.toLocal8Bit().data());

		if(device.isEmpty())
		{
			device = "default";
		}

		//EngineUtils::set_value(ret, "device", device.toLocal8Bit().data());

		spLog(Log::Info, this) << "Created alsa sink with " << device << " as output";
	}

	if(ret == nullptr)
	{
		spLog(Log::Debug, this) << "Will create auto audio sink";
		EngineUtils::createElement(&ret, "autoaudiosink", name.toLocal8Bit().data());
	}

	return ret;
}


bool Pipeline::addAndLinkElements()
{
	{ // before tee
		EngineUtils::addElements
		(
			GST_BIN(m->pipeline),
			{m->source, m->audioConvert, m->equalizer->element(), m->tee}
		);

		bool success = EngineUtils::linkElements({
			m->audioConvert, m->equalizer->element(), m->tee
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

	setPositionElement(m->source);
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
	if(EngineUtils::getState(m->pipeline) == GST_STATE_PLAYING)
	{
		//gst_element_send_event(m->pipeline, gst_event_new_eos());
	}

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

void Pipeline::enableVisualizer(bool b)
{
	m->visualizer->setEnabled(b);
}

void Pipeline::showVisualizerChanged()
{
	enableVisualizer
	(
		GetSetting(Set::Engine_ShowSpectrum) ||
		GetSetting(Set::Engine_ShowLevel)
	);
}

void Pipeline::enableBroadcasting(bool b)
{
	m->broadcaster->setEnabled(b);
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

void Pipeline::setEqualizerBand(int band, int val)
{
	m->equalizer->setBand(band, val);
}


void Pipeline::record(bool b)
{
	if(!m->streamRecorder)
	{
		m->streamRecorder = new StreamRecorderHandler(m->pipeline, m->tee);
		m->streamRecorder->init();
	}

	m->streamRecorder->setEnabled(b);
}

void Pipeline::setRecordingPath(const QString& path)
{
	m->streamRecorder->setTargetPath(path);
}

NanoSeconds Pipeline::seekRelative(double percent, NanoSeconds ref_ns)
{
	return m->seeker->seekRelative(percent, ref_ns);
}

NanoSeconds Pipeline::seekAbsolute(NanoSeconds ns)
{
	return m->seeker->seekAbsolute(ns);
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
	bool active = GetSetting(Set::Engine_SpeedActive);

	GstElement* speed = m->pitcher->element();
	if(!speed){
		return;
	}

	MilliSeconds pos_ms = positionMs();
	if(active)
	{
		Changeable::addElement(speed, m->audioConvert, m->equalizer->element());
		sppedChanged();
	}

	else
	{
		Changeable::removeElement(speed, m->audioConvert, m->equalizer->element());
	}

	if(EngineUtils::getState(m->pipeline) == GST_STATE_PLAYING)
	{
		pos_ms = std::max<MilliSeconds>(pos_ms, 0);
		m->seeker->seekNearestMs(pos_ms);
	}

	checkPosition();
}

void Pipeline::sppedChanged()
{
	m->pitcher->setSpeed
	(
		GetSetting(Set::Engine_Speed),
		GetSetting(Set::Engine_Pitch) / 440.0,
		GetSetting(Set::Engine_PreservePitch)
	);
}

void Pipeline::sinkChanged()
{
	GstElement* new_sink = createSink(GetSetting(Set::Engine_Sink));
	if(!new_sink){
		return;
	}

	replaceSink(m->playbackSink, new_sink, m->playbackVolume, m->pipeline, m->playbackBin);
	m->playbackSink = new_sink;
}

void Pipeline::checkPosition()
{
	MilliSeconds pos_ms = positionMs();
	pos_ms = std::max<MilliSeconds>(0, pos_ms);

	emit sigPositionChangedMs( pos_ms );

	checkAboutToFinish();
}

void Pipeline::checkAboutToFinish()
{
	MilliSeconds pos_ms = positionMs();
	MilliSeconds dur_ms = durationMs();
	MilliSeconds about_to_finish_time = getAboutToFinishTime();

	static bool about_to_finish = false;

	if(dur_ms < pos_ms || (dur_ms <= about_to_finish_time) || (pos_ms <= 0))
	{
		about_to_finish = false;
		return;
	}

	MilliSeconds difference = dur_ms - pos_ms;

	about_to_finish = (difference < about_to_finish_time);

	if(about_to_finish)
	{
		spLog(Log::Develop, this) << "About to finish in " << difference << ": Dur: " << dur_ms << ", Pos: " << pos_ms;

		emit sigAboutToFinishMs(difference);
	}
}


MilliSeconds Pipeline::getAboutToFinishTime() const
{
	MilliSeconds AboutToFinishTime=300;
	return std::max(fadingTimeMs(), AboutToFinishTime);
}

void Pipeline::setPositionElement(GstElement* element)
{
	m->positionElement = element;
	m->seeker->set_source(m->positionElement);
}

GstElement* Pipeline::positionElement()
{
	return m->positionElement;
}

MilliSeconds Pipeline::durationMs() const
{
	return EngineUtils::getDurationMs(m->positionElement);
}

MilliSeconds Pipeline::positionMs() const
{
	return EngineUtils::getPositionMs(m->positionElement);
}

bool Pipeline::hasElement(GstElement* e) const
{
	return EngineUtils::hasElement(GST_BIN(m->pipeline), e);
}

void Pipeline::getFadeInHandler()
{
	showVisualizerChanged();
}

void Pipeline::getFadeOutHandler()
{
	enableVisualizer(false);
}
