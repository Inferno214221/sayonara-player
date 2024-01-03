/* AudioDataProvider.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "AudioDataProvider.h"
#include "EngineUtils.h"
#include "Utils/Logger/Logger.h"

#include <QUrl>

#include <gst/gst.h>
#include <array>
#include <cmath>

namespace
{
	gboolean runThroughSpectrumStructure(GQuark fieldId, const GValue* value, gpointer data)
	{
		if(G_VALUE_HOLDS_INT(value))
		{
			const auto name = QString(g_quark_to_string(fieldId));
			if(name == "rate" && G_VALUE_HOLDS_INT(value))
			{
				auto* adp = static_cast<AudioDataProvider*>(data);
				adp->setSamplerate(g_value_get_int(value));
			}
		}

		return true;
	}

	void adpDecodebinReady(GstElement* /*source*/, GstPad* newSrcPad, gpointer data)
	{
		auto* adp = static_cast<AudioDataProvider*>(data);
		auto* audioconvert = adp->getAudioconverter();

		auto sinkPad = Engine::Utils::getStaticPad(audioconvert, Engine::Utils::sink);
		if(!*sinkPad)
		{
			return;
		}

		if(gst_pad_is_linked(*sinkPad))
		{
			return;
		}

		const auto gstPadLinkReturn = gst_pad_link(newSrcPad, *sinkPad);
		const auto* caps = gst_pad_get_current_caps(newSrcPad);
		for(auto i = 0U; i < gst_caps_get_size(caps); i++)
		{
			auto* s = gst_caps_get_structure(caps, i);
			gst_structure_foreach(s, runThroughSpectrumStructure, data);
		}

		if(gstPadLinkReturn != GST_PAD_LINK_OK)
		{
			spLog(Log::Warning, "AudioDataProvider") << "Cannot link pads";
		}

		else
		{
			spLog(Log::Debug, "AudioDataProvider") << "Pads linked";
		}
	}

	// spectrum changed
	gboolean adpSpectrumHandler(GstBus* /*bus*/, GstMessage* message, gpointer data)
	{
		auto spectrumValues = QList<float> {};
		auto* adp = static_cast<AudioDataProvider*>(data);
		const auto binCount = adp->binCount();

		// do not free structure
		const auto* structure = gst_message_get_structure(message);
		if(!structure)
		{
			return true;
		}

		const auto* structureName = gst_structure_get_name(structure);
		if(strcmp(structureName, "spectrum") != 0)
		{
			return true;
		}

		GstClockTime clockTimeNs {0};
		gst_structure_get_clock_time(structure, "timestamp", &clockTimeNs);

		const auto* magnitudes = gst_structure_get_value(structure, "magnitude");

		for(auto i = 0U; i < binCount; ++i)
		{
			const auto* mag = gst_value_list_get_value(magnitudes, i);
			spectrumValues << g_value_get_float(mag);
		}

		adp->setSpectrum(spectrumValues, static_cast<NanoSeconds>(clockTimeNs));

		return true;
	}

	void parseError(GstMessage* message)
	{
		GError* error {nullptr};
		gchar* debug {nullptr};
		gst_message_parse_error(message, &error, &debug);
		spLog(Log::Error, "AudioDataProvider") << error->message;
	}

	// check messages from bus
	gboolean adpBusStateChanged(GstBus* bus, GstMessage* msg, gpointer data)
	{
		auto* adp = static_cast<AudioDataProvider*>(data);

		const auto messageType = GST_MESSAGE_TYPE(msg);
		const auto messageSourceName = QString(GST_MESSAGE_SRC_NAME(msg)).toLower();

		switch(messageType)
		{
			case GST_MESSAGE_ELEMENT:
				if(messageSourceName.contains("spectrum"))
				{
					return adpSpectrumHandler(bus, msg, data);
				}
				break;

			case GST_MESSAGE_STATE_CHANGED:
				GstState oldSate, newState, pendingState; // NOLINT(cppcoreguidelines-init-variables)
				gst_message_parse_state_changed(msg, &oldSate, &newState, &pendingState);
				adp->setRunning(newState == GST_STATE_PLAYING);
				break;

			case GST_MESSAGE_ERROR:
				parseError(msg);
				adp->stop();
				break;

			case GST_MESSAGE_EOS:
				adp->setFinished(true);
				adp->stop();
				break;

			default:
				break;
		}

		return true;
	}
};

struct AudioDataProvider::Private
{
	GstElement* pipeline = nullptr;
	GstElement* source = nullptr;
	GstElement* audioconvert = nullptr;
	GstElement* spectrum = nullptr;
	GstElement* fakesink = nullptr;

	QString filename;
	MilliSeconds intervalMs {50}; // NOLINT(readability-magic-numbers)
	uint binCount {100U}; // NOLINT(readability-magic-numbers)
	int threshold {-75}; // NOLINT(readability-magic-numbers)
	uint samplerate {44'100U}; // NOLINT(readability-magic-numbers)
	bool isRunning {false};
	bool isFinished {false};

	explicit Private(AudioDataProvider* parent) :
		pipeline {gst_pipeline_new("adp_pipeline")}
	{
		Engine::Utils::createElement(&source, "uridecodebin", "adp_source");
		Engine::Utils::createElement(&audioconvert, "audioconvert", "adp_audioconvert");
		Engine::Utils::createElement(&spectrum, "spectrum", "adp_spectrum");
		Engine::Utils::createElement(&fakesink, "fakesink", "adp_fakesink");

		Engine::Utils::addElements(GST_BIN(pipeline), {source, audioconvert, spectrum, fakesink});

		Engine::Utils::linkElements({audioconvert, spectrum, fakesink});

		Engine::Utils::setValues(spectrum,
		                         "post-messages", true,
		                         "message-phase", false,
		                         "message-magnitude", true,
		                         "multi-channel", false
		);

		Engine::Utils::setIntValue(spectrum, "threshold", threshold);
		Engine::Utils::setUintValue(spectrum, "bands", binCount);
		Engine::Utils::setUint64Value(spectrum, "interval", guint64(intervalMs * GST_MSECOND));

		g_signal_connect (source, "pad-added", G_CALLBACK(adpDecodebinReady), parent);
	}
};

AudioDataProvider::AudioDataProvider(QObject* parent) :
	QObject(parent),
	m {Pimpl::make<Private>(this)}
{
	auto* bus = gst_pipeline_get_bus(GST_PIPELINE(m->pipeline));
	gst_bus_add_watch(bus, adpBusStateChanged, this);
	gst_object_unref(bus);
}

void AudioDataProvider::setSpectrum(const QList<float>& spectrum, NanoSeconds ns)
{
	emit sigSpectrumDataAvailable(spectrum, static_cast<MilliSeconds>(ns / 1'000'000));
}

GstElement* AudioDataProvider::getAudioconverter() const { return m->audioconvert; }

AudioDataProvider::~AudioDataProvider() = default;

void AudioDataProvider::start(const QString& filename)
{
	m->isRunning = false;
	m->isFinished = false;
	m->filename = filename;

	const auto localFile = QString("file://%1").arg(filename);
	Engine::Utils::setValue(m->source, "uri", localFile.toLocal8Bit().data());

	emit sigStarted();
	Engine::Utils::setState(m->pipeline, GST_STATE_PLAYING);
}

void AudioDataProvider::stop()
{
	Engine::Utils::setState(m->pipeline, GST_STATE_NULL);
	emit sigFinished();
}

uint AudioDataProvider::binCount() const { return m->binCount; }

void AudioDataProvider::setBinCount(uint numBins)
{
	m->binCount = numBins;
	Engine::Utils::setUintValue(m->spectrum, "bands", numBins);
}

MilliSeconds AudioDataProvider::intervalMs() const { return m->intervalMs; }

void AudioDataProvider::setIntervalMs(const MilliSeconds ms)
{
	m->intervalMs = ms;
	Engine::Utils::setUint64Value(m->spectrum, "interval", guint64(ms * GST_MSECOND));
}

int AudioDataProvider::threshold() const { return m->threshold; }

void AudioDataProvider::setThreshold(const int threshold)
{
	m->threshold = threshold;
	Engine::Utils::setIntValue(m->spectrum, "threshold", threshold);
}

void AudioDataProvider::setSamplerate(const uint samplerate) { m->samplerate = samplerate; }

uint AudioDataProvider::samplerate() const { return m->samplerate; }

float AudioDataProvider::frequency(const int bins)
{
	return ((m->samplerate / 2.0F) * bins + m->samplerate / 4.0F) / m->binCount;
}

void AudioDataProvider::setRunning(const bool b) { m->isRunning = b; }

bool AudioDataProvider::isRunning() const { return m->isRunning; }

void AudioDataProvider::setFinished(const bool b) { m->isFinished = b; }

bool AudioDataProvider::isFinished(const QString& filename) const
{
	return ((m->filename == filename) && m->isFinished);
}

