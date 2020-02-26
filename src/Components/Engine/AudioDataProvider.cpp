#include "AudioDataProvider.h"
#include "EngineUtils.h"
#include "Utils/Logger/Logger.h"

#include <QUrl>

#include <gst/gst.h>
#include <array>
#include <cmath>

namespace EngineUtils=::Engine::Utils;

static
bool runThroughSpectrumStructure(GQuark field_id, const GValue* value, gpointer data)
{
	if(G_VALUE_HOLDS_INT(value))
	{
		QString name(g_quark_to_string(field_id));
		if(name == "rate" && G_VALUE_HOLDS_INT(value))
		{
			auto* adp = static_cast<AudioDataProvider*>(data);
			adp->setSamplerate(g_value_get_int(value));
		}
	}

	return true;
}

static
void adpDecodebinReady(GstElement* source, GstPad* new_src_pad, gpointer data)
{
	Q_UNUSED(source)

	auto* adp = static_cast<AudioDataProvider*>(data);
	GstElement* audioconvert = adp->getAudioconverter();

	GstPad*	sink_pad = gst_element_get_static_pad(audioconvert, "sink");
	if(!sink_pad){
		return;
	}

	if(gst_pad_is_linked(sink_pad))
	{
		gst_object_unref(sink_pad);
		return;
	}

	GstPadLinkReturn gplr = gst_pad_link(new_src_pad, sink_pad);
	GstCaps* caps = gst_pad_get_current_caps(new_src_pad);
	for(guint i=0; i<gst_caps_get_size(caps); i++)
	{
		GstStructure* s = gst_caps_get_structure(caps, i);
		gst_structure_foreach(s, GstStructureForeachFunc(runThroughSpectrumStructure), data);

	}

	if(gplr != GST_PAD_LINK_OK){
		spLog(Log::Warning, "AudioDataProvider") << "Cannot link pads";
	}

	else {
		spLog(Log::Debug, "AudioDataProvider") << "Pads linked";
	}
}


// spectrum changed
gboolean
adpSpectrumHandler(GstBus* bus, GstMessage* message, gpointer data)
{
	Q_UNUSED(bus)
	QList<float> spectrum_vals;

	auto* adp = static_cast<AudioDataProvider*>(data);
	const uint binCount = adp->binCount();

	// do not free structure
	const GstStructure* structure = gst_message_get_structure(message);
	if(!structure) {
		return true;
	}

	const gchar* structure_name = gst_structure_get_name(structure);
	if( strcmp(structure_name, "spectrum") != 0 ) {
		return true;
	}

	GstClockTime clock_time_ns;
	gst_structure_get_clock_time(structure, "timestamp", &clock_time_ns);

	const GValue* magnitudes = gst_structure_get_value(structure, "magnitude");

	for(guint i=0; i<binCount; ++i)
	{
		const GValue* mag = gst_value_list_get_value(magnitudes, i);
		float f = g_value_get_float(mag);
		spectrum_vals << (f);
	}

	adp->setSpectrum(spectrum_vals, NanoSeconds(clock_time_ns));

	return true;
}


// check messages from bus
static
gboolean adpBusStateChanged(GstBus* bus, GstMessage* msg, gpointer data)
{
	auto* adp = static_cast<AudioDataProvider*>(data);

	GstMessageType msg_type = GST_MESSAGE_TYPE(msg);
	QString	msg_src_name = QString(GST_MESSAGE_SRC_NAME(msg)).toLower();

	switch (msg_type)
	{
		case GST_MESSAGE_ELEMENT:
			if(msg_src_name.contains("spectrum")){
				return adpSpectrumHandler(bus, msg, data);
			}
			break;

		case GST_MESSAGE_STATE_CHANGED:
			GstState oldSate, newState, pendingState;
			gst_message_parse_state_changed(msg, &oldSate, &newState, &pendingState);
			adp->setRunning(newState == GST_STATE_PLAYING);
			break;

		case GST_MESSAGE_ERROR:
			GError* error;
			gchar* debug;
			gst_message_parse_error(msg, &error, &debug);

			spLog(Log::Error, "AudioDataProvider") << error->message;
			adp->stop();
			break;

		case GST_MESSAGE_EOS:
			adp->setFinished(true);
			adp->stop();
			break;

		default: break;
	}

	return true;
}


struct AudioDataProvider::Private
{
	GstElement* pipeline=nullptr;
	GstElement* source=nullptr;
	GstElement* audioconvert=nullptr;
	GstElement* spectrum=nullptr;
	GstElement* fakesink=nullptr;

	QString	filename;
	MilliSeconds intervalMs;
	uint binCount;
	int threshold;
	uint samplerate;
	bool isRunning;
	bool isFinished;

	Private(AudioDataProvider* parent) :
		intervalMs(50),
		binCount(100),
		threshold(-75),
		samplerate(44100),
		isRunning(false),
		isFinished(false)
	{
		pipeline = gst_pipeline_new("adp_pipeline");

		EngineUtils::createElement(&source, "uridecodebin", "adp_source");
		EngineUtils::createElement(&audioconvert, "audioconvert", "adp_audioconvert");
		EngineUtils::createElement(&spectrum, "spectrum", "adp_spectrum");
		EngineUtils::createElement(&fakesink, "fakesink", "adp_fakesink");

		EngineUtils::addElements(GST_BIN(pipeline),
			{source, audioconvert, spectrum, fakesink}
		);

		EngineUtils::linkElements(
			{audioconvert, spectrum, fakesink}
		);

		EngineUtils::setValues(spectrum,
			"post-messages", true,
			"message-phase", false,
			"message-magnitude", true,
			"multi-channel", false
		);

		Engine::Utils::setIntValue(spectrum, "threshold", threshold);
		EngineUtils::setUintValue(spectrum, "bands", binCount);
		Engine::Utils::setUint64Value(spectrum, "interval", guint64(intervalMs * GST_MSECOND));

		g_signal_connect (source, "pad-added", G_CALLBACK(adpDecodebinReady), parent);
	}
};

AudioDataProvider::AudioDataProvider(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(this);

	GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(m->pipeline));
	gst_bus_add_watch(bus, adpBusStateChanged, this);
	gst_object_unref(bus);
}

void AudioDataProvider::setSpectrum(const QList<float>& spectrum, NanoSeconds ns)
{
	emit sigSpectrumDataAvailable(spectrum, MilliSeconds(ns / 1000000));
}

GstElement* AudioDataProvider::getAudioconverter() const
{
	return m->audioconvert;
}

AudioDataProvider::~AudioDataProvider() = default;

void AudioDataProvider::start(const QString& filename)
{
	m->isRunning = false;
	m->isFinished = false;
	m->filename = filename;

	QString local_file = "file://" + filename;
	EngineUtils::setValue(m->source, "uri", local_file.toLocal8Bit().data());

	emit sigStarted();
	EngineUtils::setState(m->pipeline, GST_STATE_PLAYING);
}

void AudioDataProvider::stop()
{
	EngineUtils::setState(m->pipeline, GST_STATE_NULL);
	emit sigFinished();
}

uint AudioDataProvider::binCount() const
{
	return m->binCount;
}

void AudioDataProvider::setBinCount(uint num_bins)
{
	m->binCount = num_bins;
	EngineUtils::setUintValue(m->spectrum, "bands", num_bins);
}

MilliSeconds AudioDataProvider::intervalMs() const
{
	return m->intervalMs;
}

void AudioDataProvider::setIntervalMs(MilliSeconds ms)
{
	m->intervalMs = ms;
	Engine::Utils::setUint64Value(m->spectrum, "interval", guint64(ms * GST_MSECOND));
}

int AudioDataProvider::threshold() const
{
	return m->threshold;
}

void AudioDataProvider::setThreshold(int threshold)
{
	m->threshold = threshold;
	Engine::Utils::setIntValue(m->spectrum, "threshold", threshold);
}

void AudioDataProvider::setSamplerate(uint samplerate)
{
	m->samplerate = samplerate;
}

uint AudioDataProvider::samplerate() const
{
	return m->samplerate;
}

float AudioDataProvider::frequency(int bin)
{
	return ((m->samplerate / 2.0f) * bin + m->samplerate / 4.0f) / m->binCount;
}

void AudioDataProvider::setRunning(bool b)
{
	m->isRunning = b;
}

bool AudioDataProvider::isRunning() const
{
	return m->isRunning;
}

void AudioDataProvider::setFinished(bool b)
{
	m->isFinished = b;
}

bool AudioDataProvider::isFinished(const QString& filename) const
{
	return ((m->filename == filename) && m->isFinished);
}

