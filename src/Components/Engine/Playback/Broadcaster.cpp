#include "Broadcaster.h"
#include "PipelineProbes.h"

#include "Callbacks/EngineUtils.h"
#include "Callbacks/PipelineCallbacks.h"

#include <QString>
#include <QList>

struct Broadcaster::Private
{
	GstElement* pipeline=nullptr;
	GstElement* tee=nullptr;

	GstElement*			bc_bin=nullptr;
	GstElement*			bc_queue=nullptr;
	GstElement*			bc_converter=nullptr;
	GstElement*			bc_resampler=nullptr;
	GstElement*			bc_lame=nullptr;
	GstElement*			bc_app_sink=nullptr;

	gulong				bc_probe;

	bool				is_running;

	Private(GstElement* pipeline, GstElement* tee) :
		pipeline(pipeline),
		tee(tee),
		bc_probe(0),
		is_running(false)
	{}
};

Broadcaster::Broadcaster(GstElement* pipeline, GstElement* tee)
{
	m = Pimpl::make<Private>(pipeline, tee);
}

Broadcaster::~Broadcaster() {}

bool Broadcaster::init()
{
	if(m->bc_bin){
		return true;
	}

	// create
	if( !Engine::Utils::create_element(&m->bc_queue, "queue", "lame_queue") ||
		!Engine::Utils::create_element(&m->bc_converter, "audioconvert", "lame_converter") ||
		!Engine::Utils::create_element(&m->bc_resampler, "audioresample", "lame_resampler") ||
		!Engine::Utils::create_element(&m->bc_lame, "lamemp3enc") ||
		!Engine::Utils::create_element(&m->bc_app_sink, "appsink", "lame_appsink"))
	{
		return false;
	}

	{ // init bin
		bool success = Engine::Utils::create_bin(&m->bc_bin, {m->bc_queue,  m->bc_converter, m->bc_resampler, m->bc_lame, m->bc_app_sink}, "broadcast");
		if(!success){
			return false;
		}

		gst_bin_add(GST_BIN(m->pipeline), m->bc_bin);
		success = Engine::Utils::tee_connect(m->tee, m->bc_bin, "BroadcastQueue");
		if(!success){
			Engine::Utils::set_state(m->bc_bin, GST_STATE_NULL);
			gst_object_unref(m->bc_bin);
			return false;
		}
	}

	{ // configure
		gst_object_ref(m->bc_app_sink);

		Engine::Utils::config_lame(m->bc_lame);
		Engine::Utils::config_queue(m->bc_queue);
		Engine::Utils::config_sink(m->bc_app_sink);
		Engine::Utils::set_values(G_OBJECT(m->bc_app_sink), "emit-signals", true);

		g_signal_connect (m->bc_app_sink, "new-sample", G_CALLBACK(Pipeline::Callbacks::new_buffer), this);
	}

	return true;
}

bool Broadcaster::set_enabled(bool b)
{
	if(b && !init()){
		return false;
	}

	m->is_running = b;
	Pipeline::Probing::handle_probe(&m->is_running, m->bc_queue, &m->bc_probe, Pipeline::Probing::lame_probed);

	return true;
}
