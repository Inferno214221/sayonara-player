#include "Visualizer.h"
#include "PipelineProbes.h"
#include "EngineUtils.h"

#include "Utils/Settings/Settings.h"

#include <QString>
#include <QList>

using namespace PipelineExtensions;

struct Visualizer::Private
{
	GstElement* pipeline=nullptr;
	GstElement* tee=nullptr;

	GstElement*			visualizer_bin=nullptr;
	GstElement*			visualizer_queue=nullptr;
	GstElement*			spectrum=nullptr;
	GstElement*			level=nullptr;
	GstElement*			visualizer_sink=nullptr;

	gulong				probe;
	bool				is_running;

	Private(GstElement* pipeline, GstElement* tee) :
		pipeline(pipeline),
		tee(tee),
		probe(0),
		is_running(false)
	{}
};

Visualizer::Visualizer(GstElement* pipeline, GstElement* tee)
{
	m = Pimpl::make<Private>(pipeline, tee);
}

Visualizer::~Visualizer() {}

bool Visualizer::init()
{

	{ // create
		if(	EngineUtils::create_element(&m->visualizer_queue, "queue", "visualizer") &&
			EngineUtils::create_element(&m->level, "level") &&	// in case of renaming, also look in EngineCallbase GST_MESSAGE_EVENT
			EngineUtils::create_element(&m->spectrum, "spectrum") &&
			EngineUtils::create_element(&m->visualizer_sink,"fakesink", "visualizer"))
		{
			EngineUtils::create_bin(&m->visualizer_bin, {m->visualizer_queue, m->level, m->spectrum, m->visualizer_sink}, "visualizer");
		}

		if(!m->visualizer_bin){
			return false;
		}
	}

	{ // link
		gst_bin_add(GST_BIN(m->pipeline), m->visualizer_bin);
		bool success = EngineUtils::tee_connect(m->tee, m->visualizer_bin, "Visualizer");
		if(!success)
		{
			gst_bin_remove(GST_BIN(m->pipeline), m->visualizer_bin);
			gst_object_unref(m->visualizer_bin);
			m->visualizer_bin = nullptr;
			return false;
		}
	}

	{ // configure
		EngineUtils::set_values(G_OBJECT(m->level), "post-messages", true);
		EngineUtils::set_uint64_value(G_OBJECT(m->level), "interval", 20 * GST_MSECOND);
		EngineUtils::set_values(G_OBJECT (m->spectrum),
					  "post-messages", true,
					  "message-phase", false,
					  "message-magnitude", true,
					  "multi-channel", false);

		EngineUtils::set_int_value(G_OBJECT(m->spectrum), "threshold", -75);
		EngineUtils::set_uint_value(G_OBJECT(m->spectrum), "bands", GetSetting(Set::Engine_SpectrumBins));
		EngineUtils::set_uint64_value(G_OBJECT(m->spectrum), "interval", 20 * GST_MSECOND);

		EngineUtils::config_queue(m->visualizer_queue, 1000);
		EngineUtils::config_sink(m->visualizer_sink);
	}

	return true;
}

bool Visualizer::set_enabled(bool b)
{
	if(!init()){
		return false;
	}

	m->is_running = b;
	Probing::handle_probe(&m->is_running, m->visualizer_queue, &m->probe, Probing::spectrum_probed);

	bool show_level = GetSetting(Set::Engine_ShowLevel);
	bool show_spectrum = GetSetting(Set::Engine_ShowSpectrum);

	EngineUtils::set_value(G_OBJECT(m->level), "post-messages", show_level);
	EngineUtils::set_value(G_OBJECT(m->spectrum), "post-messages", show_spectrum);

	return true;
}
