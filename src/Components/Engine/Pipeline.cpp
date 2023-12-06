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
#include <utility>

namespace Engine
{
	struct Pipeline::Private
	{
		QString name;

		GstElement* pipeline = nullptr;
		GstElement* source = nullptr;
		GstElement* audioConvert = nullptr;
		GstElement* pitch = nullptr;
		GstElement* equalizer = nullptr;
		GstElement* tee = nullptr;

		GstElement* playbackBin = nullptr;
		GstElement* playbackQueue = nullptr;
		GstElement* playbackVolume = nullptr;
		GstElement* playbackSink = nullptr;

		PipelineExtensions::StreamRecorderBin* streamRecorder = nullptr;
		std::shared_ptr<PipelineExtensions::Broadcaster> broadcaster = nullptr;
		std::shared_ptr<PipelineExtensions::RawDataReceiver> rawDataReceiver = nullptr;
		std::shared_ptr<PipelineExtensions::VisualizerBin> visualizer = nullptr;

		QTimer* progressTimer = nullptr;

		explicit Private(QString name) :
			name(std::move(name)) {}

		GstElement* createSink(const QString& sinkName)
		{
			static int Number = 1;

			GstElement* ret = nullptr;

			const auto newName = sinkName + QString::number(Number);

			if(sinkName == "pulse")
			{
				spLog(Log::Debug, this) << "Create pulseaudio sink";
				Utils::createElement(&ret, "pulsesink", newName.toLocal8Bit().data());
			}

			else if(sinkName == "alsa")
			{
				spLog(Log::Debug, this) << "Create alsa sink";
				auto device = GetSetting(Set::Engine_AlsaDevice);
				Utils::createElement(&ret, "alsasink", newName.toLocal8Bit().data());

				if(device.isEmpty())
				{
					device = "default";
				}

				spLog(Log::Info, this) << "Created alsa sink with " << device << " as output";
			}

			if(!ret)
			{
				spLog(Log::Debug, this) << "Will create auto audio sink";
				Utils::createElement(&ret, "autoaudiosink", newName.toLocal8Bit().data());
			}

			return ret;
		}
	};

	Pipeline::Pipeline(const QString& name, QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>(name)} {}

	Pipeline::~Pipeline()
	{
		if(m->pipeline)
		{
			Utils::setState(m->pipeline, GST_STATE_NULL);
			gst_object_unref(GST_OBJECT(m->pipeline));
			m->pipeline = nullptr;
		}
	}

	bool Pipeline::init(Engine* engine)
	{
		if(m->pipeline)
		{
			return true;
		}

		m->pipeline = gst_pipeline_new(m->name.toStdString().c_str());
		if(!Utils::testAndError(m->pipeline, "Engine: Pipeline sucks"))
		{
			return false;
		}

		if(!createElements())
		{
			return false;
		}

		if(!addAndLinkElements())
		{
			return false;
		}

		configureElements();
		Utils::setState(m->pipeline, GST_STATE_NULL);

		auto* bus = gst_pipeline_get_bus(GST_PIPELINE(m->pipeline));
		gst_bus_add_watch(bus, Callbacks::busStateChanged, engine);

		m->progressTimer = new QTimer(this);
		m->progressTimer->setTimerType(Qt::PreciseTimer);
		m->progressTimer->setInterval(Utils::getUpdateInterval()); // NOLINT(bugprone-narrowing-conversions)
		connect(m->progressTimer, &QTimer::timeout, this, [this]() {
			if(Utils::getState(m->pipeline) != GST_STATE_NULL)
			{
				Callbacks::positionChanged(this);
			}
		});

		m->progressTimer->start();

		SetSetting(SetNoDB::MP3enc_found, Utils::isLameAvailable());
		SetSetting(SetNoDB::Pitch_found, Utils::isPitchAvailable());

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
		if(!Utils::createElement(&m->source, "uridecodebin", "src")) { return false; }
		if(!Utils::createElement(&m->audioConvert, "audioconvert")) { return false; }
		if(!Utils::createElement(&m->tee, "tee")) { return false; }
		if(!Utils::createElement(&m->playbackQueue, "queue", "playback_queue")) { return false; }
		if(!Utils::createElement(&m->playbackVolume, "volume")) { return false; }

		// optional pitch
		//Utils::createElement(&m->pitch, "pitch");

		Utils::createElement(&m->equalizer, "equalizer-10bands");

		m->playbackSink = m->createSink(GetSetting(Set::Engine_Sink));

		m->visualizer = PipelineExtensions::createVisualizerBin(m->pipeline, m->tee);
		m->rawDataReceiver = PipelineExtensions::createRawDataReceiver([this](const auto& data) {
			emit sigDataAvailable(data);
		});

		m->broadcaster = PipelineExtensions::createBroadcaster(m->rawDataReceiver, m->pipeline, m->tee);

		return (m->playbackSink != nullptr);
	}

	bool Pipeline::addAndLinkElements()
	{
		{ // before tee
			Utils::addElements(GST_BIN(m->pipeline),
			                   {m->source, m->audioConvert, m->equalizer, m->tee});

			const auto success = Utils::linkElements({m->audioConvert, m->equalizer, m->tee});

			if(!Utils::testAndErrorBool(success, "Engine: Cannot link audio convert with tee"))
			{
				return false;
			}
		}

		{ // Playback Bin
			m->playbackBin = gst_bin_new("Playback_bin");
			Utils::addElements(GST_BIN(m->playbackBin), {m->playbackQueue, m->playbackVolume, m->playbackSink});

			const auto success = Utils::linkElements({m->playbackQueue, m->playbackVolume, m->playbackSink});
			if(!Utils::testAndErrorBool(success, "Engine: Cannot link eq with audio sink"))
			{
				return false;
			}

			Utils::addElements(GST_BIN(m->pipeline), {m->playbackBin});
			Utils::createGhostPad(GST_BIN(m->playbackBin), m->playbackQueue);
			Utils::connectTee(m->tee, m->playbackBin, "Equalizer");

			return Utils::testAndErrorBool(success, "Engine: Cannot link eq queue with tee");
		}
	}

	void Pipeline::configureElements()
	{
		Utils::setValues(G_OBJECT(m->tee), "silent", true, "allow-not-linked", true);

		Utils::configureQueue(m->playbackQueue);

		g_signal_connect (m->source, "pad-added", G_CALLBACK(Callbacks::decodebinReady), m->audioConvert);
		g_signal_connect (m->source, "source-setup", G_CALLBACK(Callbacks::sourceReady), nullptr);
	}

	bool Pipeline::prepare(const QString& uri)
	{
		stop();

		Utils::setValues(G_OBJECT(m->source), "use-buffering", Util::File::isWWW(uri), "uri", uri.toUtf8().data());
		Utils::setInt64Value(G_OBJECT(m->source), "buffer-duration", GetSetting(Set::Engine_BufferSizeMS));
		Utils::setState(m->pipeline, GST_STATE_PAUSED);

		volumeChanged();
		muteChanged();

		return true;
	}

	void Pipeline::play()
	{
		Utils::setState(m->pipeline, GST_STATE_PLAYING);
	}

	void Pipeline::pause()
	{
		Utils::setState(m->pipeline, GST_STATE_PAUSED);
	}

	void Pipeline::stop()
	{
		Utils::setState(m->pipeline, GST_STATE_NULL);

		abortDelayedPlaying();
		abortFader();
	}

	void Pipeline::volumeChanged()
	{
		const auto volume = GetSetting(Set::Engine_Vol) / 100.0;
		setInternalVolume(volume);
	}

	void Pipeline::muteChanged()
	{
		const auto muted = GetSetting(Set::Engine_Mute);
		Utils::setValue(G_OBJECT(m->playbackVolume), "mute", muted);
	}

	void Pipeline::setVisualizerEnabled(bool levelEnabled, bool spectrumEnabled)
	{
		m->visualizer->setEnabled(levelEnabled, spectrumEnabled);
	}

	bool Pipeline::isLevelVisualizerEnabled() const { return m->visualizer->isLevelEnabled(); }

	bool Pipeline::isSpectrumVisualizerEnabled() const { return m->visualizer->isSpectrumEnabled(); }

	void Pipeline::setBroadcastingEnabled(const bool b) { m->broadcaster->setEnabled(b); }

	bool Pipeline::isBroadcastingEnabled() const { return m->broadcaster->isEnabled(); }

	GstState Pipeline::state() const { return Utils::getState(m->pipeline); }

	MilliSeconds Pipeline::timeToGo() const
	{
		auto* element = m->pipeline;
		const auto ms = Utils::getTimeToGo(element);

		return std::max<MilliSeconds>(ms - 100, 0); // NOLINT(readability-magic-numbers)
	}

	void Pipeline::prepareForRecording()
	{
		if(!m->streamRecorder)
		{
			m->streamRecorder = PipelineExtensions::StreamRecorderBin::create(m->pipeline, m->tee);
			m->streamRecorder->init();
		}
	}

	void Pipeline::finishRecording()
	{
		m->streamRecorder->setTargetPath({});
	}

	void Pipeline::setRecordingPath(const QString& path)
	{
		m->streamRecorder->setTargetPath(path);
	}

	void Pipeline::setInternalVolume(const double volume)
	{
		Utils::setValue(G_OBJECT(m->playbackVolume), "volume", volume);
	}

	double Pipeline::internalVolume() const
	{
		double volume; // NOLINT(cppcoreguidelines-init-variables)
		g_object_get(m->playbackVolume, "volume", &volume, nullptr); // NOLINT(cppcoreguidelines-pro-type-vararg)
		return volume;
	}

	void Pipeline::speedActiveChanged()
	{
		if(!m->pitch)
		{
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

		if(Utils::getState(m->pipeline) == GST_STATE_PLAYING)
		{
			const auto positionMs = std::max<MilliSeconds>(this->positionMs(), 0);
			seekNearestMs(positionMs);
		}

		checkPosition();
	}

	void Pipeline::sppedChanged()
	{
//	Pitchable::setSpeed(
//		GetSetting(Set::Engine_Speed),
//		GetSetting(Set::Engine_Pitch) / 440.0,
//		GetSetting(Set::Engine_PreservePitch));
	}

	void Pipeline::sinkChanged()
	{
		auto* newSink = m->createSink(GetSetting(Set::Engine_Sink));
		if(newSink)
		{
			replaceSink(m->playbackSink, newSink, m->playbackVolume, m->pipeline, m->playbackBin);
			m->playbackSink = newSink;
		}
	}

	void Pipeline::checkPosition()
	{
		emit sigPositionChangedMs(std::max<MilliSeconds>(0, positionMs()));

		checkAboutToFinish();
	}

	void Pipeline::checkAboutToFinish()
	{
		const auto positionMs = this->positionMs();
		const auto durationMs = this->durationMs();
		const auto aboutToFinishMs = std::max<MilliSeconds>(fadingTimeMs(), 300);

		static bool aboutToFinish = false;

		if(durationMs < positionMs || (durationMs <= aboutToFinishMs) || (positionMs <= 0))
		{
			aboutToFinish = false;
			return;
		}

		const auto difference = durationMs - positionMs;

		aboutToFinish = (difference < aboutToFinishMs);
		if(aboutToFinish)
		{
			spLog(Log::Develop, this) << "About to finish in " << difference << ": Dur: " << durationMs << ", Pos: "
			                          << positionMs;
			emit sigAboutToFinishMs(difference);
		}
	}

	bool Pipeline::hasElement(GstElement* e) const { return Utils::hasElement(GST_BIN(m->pipeline), e); }

	GstElement* Pipeline::positionElement() const { return m->playbackSink; }

//GstElement* Pipeline::pitchElement() const
//{
//	return m->pitch;
//}

	GstElement* Pipeline::equalizerElement() const { return m->equalizer; }

	void Pipeline::postProcessFadeIn() {}

	void Pipeline::postProcessFadeOut() {}
}