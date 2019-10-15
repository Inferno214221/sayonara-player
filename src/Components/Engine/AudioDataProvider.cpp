#include "AudioDataProvider.h"
#include "EngineUtils.h"
#include "Utils/Logger/Logger.h"

#include <QUrl>

#include <gst/gst.h>
#include <array>
#include <cmath>

namespace EngineUtils=::Engine::Utils;

static
bool run_through_structure(GQuark field_id, const GValue* value, gpointer data)
{
	if(G_VALUE_HOLDS_INT(value))
	{
		QString name(g_quark_to_string(field_id));
		if(name == "rate" && G_VALUE_HOLDS_INT(value))
		{
			auto* adp = static_cast<AudioDataProvider*>(data);
			adp->set_samplerate(g_value_get_int(value));
		}
	}

	return true;
}

static
void adp_decodebin_ready(GstElement* source, GstPad* new_src_pad, gpointer data)
{
	Q_UNUSED(source)

	auto* adp = static_cast<AudioDataProvider*>(data);
	GstElement* audioconvert = adp->get_audioconvert();

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
		gst_structure_foreach(s, GstStructureForeachFunc(run_through_structure), data);

	}

	if(gplr != GST_PAD_LINK_OK){
		sp_log(Log::Warning, "AudioDataProvider") << "Cannot link pads";
	}

	else {
		sp_log(Log::Debug, "AudioDataProvider") << "Pads linked";
	}
}


// spectrum changed
gboolean
adp_spectrum_handler(GstBus* bus, GstMessage* message, gpointer data)
{
	Q_UNUSED(bus);
	QList<float> spectrum_vals;

	auto* adp = static_cast<AudioDataProvider*>(data);
	const uint num_bins = adp->get_number_bins();

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

	for(guint i=0; i<num_bins; ++i)
	{
		const GValue* mag = gst_value_list_get_value(magnitudes, i);
		float f = g_value_get_float(mag);
		spectrum_vals << (f);
	}

	adp->set_spectrum(spectrum_vals, NanoSeconds(clock_time_ns));

	return true;
}


// check messages from bus
static
gboolean adp_bus_state_changed(GstBus* bus, GstMessage* msg, gpointer data)
{
	auto* adp = static_cast<AudioDataProvider*>(data);

	GstMessageType msg_type = GST_MESSAGE_TYPE(msg);
	QString	msg_src_name = QString(GST_MESSAGE_SRC_NAME(msg)).toLower();

	switch (msg_type)
	{
		case GST_MESSAGE_ELEMENT:
			if(msg_src_name.contains("spectrum")){
				return adp_spectrum_handler(bus, msg, data);
			}
			break;

		case GST_MESSAGE_ERROR:
			GError* error;
			gchar* debug;
			gst_message_parse_error(msg, &error, &debug);

			sp_log(Log::Error, "AudioDataProvider") << error->message;
			adp->stop();
			break;
		case GST_MESSAGE_EOS:
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

	MilliSeconds interval_ms;
	uint num_bins;
	int threshold;
	uint samplerate;

	Private(AudioDataProvider* parent) :
		interval_ms(50),
		num_bins(100),
		threshold(-75),
		samplerate(44100)
	{
		pipeline = gst_pipeline_new("adp_pipeline");

		EngineUtils::create_element(&source, "uridecodebin", "adp_source");
		EngineUtils::create_element(&audioconvert, "audioconvert", "adp_audioconvert");
		EngineUtils::create_element(&spectrum, "spectrum", "adp_spectrum");
		EngineUtils::create_element(&fakesink, "fakesink", "adp_fakesink");

		EngineUtils::add_elements(GST_BIN(pipeline),
			{source, audioconvert, spectrum, fakesink}
		);

		EngineUtils::link_elements(
			{audioconvert, spectrum, fakesink}
		);

		EngineUtils::set_values(spectrum,
			"post-messages", true,
			"message-phase", false,
			"message-magnitude", true,
			"multi-channel", false
		);

		Engine::Utils::set_int_value(spectrum, "threshold", threshold);
		EngineUtils::set_uint_value(spectrum, "bands", num_bins);
		Engine::Utils::set_uint64_value(spectrum, "interval", interval_ms * GST_MSECOND);

		g_signal_connect (source, "pad-added", G_CALLBACK(adp_decodebin_ready), parent);
	}
};

AudioDataProvider::AudioDataProvider(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(this);

	GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(m->pipeline));
	gst_bus_add_watch(bus, adp_bus_state_changed, this);
	gst_object_unref(bus);
}

void AudioDataProvider::set_spectrum(const QList<float>& spectrum, NanoSeconds ns)
{
	emit sig_spectrum(spectrum, MilliSeconds(ns / 1000000));
}

GstElement* AudioDataProvider::get_audioconvert() const
{
	return m->audioconvert;
}

AudioDataProvider::~AudioDataProvider() = default;

void AudioDataProvider::set_filename(const QString& name)
{
	QString local_file = "file://" + name;

	EngineUtils::set_value(m->source, "uri", local_file.toLocal8Bit().data());
}

void AudioDataProvider::start()
{
	emit sig_started();
	EngineUtils::set_state(m->pipeline, GST_STATE_PLAYING);
}

void AudioDataProvider::stop()
{
	EngineUtils::set_state(m->pipeline, GST_STATE_NULL);
	emit sig_finished();
}

uint AudioDataProvider::get_number_bins() const
{
	return m->num_bins;
}

void AudioDataProvider::set_number_bins(uint num_bins)
{
	m->num_bins = num_bins;
	EngineUtils::set_uint_value(m->spectrum, "bands", num_bins);
}

MilliSeconds AudioDataProvider::get_interval_ms() const
{
	return m->interval_ms;
}

void AudioDataProvider::set_interval_ms(MilliSeconds ms)
{
	m->interval_ms = ms;
	Engine::Utils::set_uint64_value(m->spectrum, "interval", guint64(ms * GST_MSECOND));
}

int AudioDataProvider::get_threshold() const
{
	return m->threshold;
}

void AudioDataProvider::set_threshold(int threshold)
{
	m->threshold = threshold;
	Engine::Utils::set_int_value(m->spectrum, "threshold", threshold);
}

void AudioDataProvider::set_samplerate(uint samplerate)
{
	m->samplerate = samplerate;
}

uint AudioDataProvider::get_samplerate() const
{
	return m->samplerate;
}

float AudioDataProvider::get_frequency(int bin)
{
	return ((m->samplerate / 2.0f) * bin + m->samplerate / 4.0f) / m->num_bins;
}
