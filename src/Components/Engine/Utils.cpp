/* Engine::Utils.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "Utils.h"
#include "Utils/Logger/Logger.h"

#include <QString>
#include <QStringList>

#include <gst/base/gstbasetransform.h>

struct TeeProbeData
{
	GstState	state;
	GstElement* element;
};

static GstPadProbeReturn
tee_probe_blocked(GstPad* pad, GstPadProbeInfo* info, gpointer p)
{
	TeeProbeData* data = static_cast<TeeProbeData*>(p);
	GstElement* queue = data->element;

	if(!Engine::Utils::test_and_error(queue, "Connect to tee: Element is not GstElement")){
		delete data; data = nullptr;
		return GST_PAD_PROBE_DROP;
	}

	GstPad* queue_pad = gst_element_get_static_pad(queue, "sink");
	if(!Engine::Utils::test_and_error(queue_pad, "Connect to tee: No valid pad from GstElement")){
		delete data; data = nullptr;
		return GST_PAD_PROBE_DROP;
	}

	GstPadLinkReturn s = gst_pad_link(pad, queue_pad);
	if(s != GST_PAD_LINK_OK) {
		sp_log(Log::Warning, "AbstractPipeline") << "Could not dynamically connect tee";
	}


	gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));
	gst_element_set_state(queue, data->state);

	delete data; data = nullptr;
	return GST_PAD_PROBE_DROP;
}

bool Engine::Utils::tee_connect(GstElement* tee, GstElement* queue, const QString& queue_name)
{
	if(!test_and_error(tee, "tee connect: tee is null")){
		return false;
	}

	if(!test_and_error(queue, "tee connect: queue is null")){
		return false;
	}

	QString error_1 = QString("Engine: Tee-") + queue_name + " pad is nullptr";
	QString error_2 = QString("Engine: ") + queue_name + " pad is nullptr";
	QString error_3 = QString("Engine: Cannot link tee with ") + queue_name;

	GstPadTemplate* tee_src_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(tee), "src_%u");
	if(!test_and_error(tee_src_pad_template, "Engine: _tee_src_pad_template is nullptr")) {
		return false;
	}

	GstPad* tee_queue_pad = gst_element_request_pad(tee, tee_src_pad_template, nullptr, nullptr);
	if(!test_and_error(tee_queue_pad, error_1)){
		return false;
	}

	GstState state	= Engine::Utils::get_state(tee);

	if(state == GST_STATE_PLAYING || state == GST_STATE_PAUSED)
	{
		TeeProbeData* data = new TeeProbeData();
		data->state = state;
		data->element = queue;

		gulong id = gst_pad_add_probe(tee_queue_pad,
				GST_PAD_PROBE_TYPE_IDLE,
				tee_probe_blocked,
				data,
				nullptr);

		Q_UNUSED(id)

		return true;
	}

	GstPad* queue_pad = gst_element_get_static_pad(queue, "sink");
	if(!test_and_error(queue_pad, error_2)) {
		return false;
	}

	GstPadLinkReturn s = gst_pad_link (tee_queue_pad, queue_pad);
	if(!test_and_error_bool((s == GST_PAD_LINK_OK), error_3)) {
		return false;
	}

	set_state(queue, get_state(tee));

	gst_object_unref(tee_queue_pad);
	gst_object_unref(queue_pad);
	return true;
}

bool Engine::Utils::has_element(GstBin* bin, GstElement* element)
{
	if(!bin || !element){
		return true;
	}

	if(!GST_IS_OBJECT(bin) || !GST_IS_OBJECT(element)){
		return false;
	}

	GstObject* o = GST_OBJECT(element);
	GstObject* parent = nullptr;

	while(o && GST_IS_OBJECT(o))
	{
		if(o == GST_OBJECT(bin))
		{
			if( o != GST_OBJECT(element) )
			{
				gst_object_unref(o);
			}

			return true;
		}

		parent = gst_object_get_parent(o);
		if( o != GST_OBJECT(element) )
		{
			gst_object_unref(o);
		}

		o = parent;
	}

	return false;
}


bool Engine::Utils::test_and_error(void* element, const QString& errorstr)
{
	if(!element) {
		sp_log(Log::Error, "Engine::Utils") << errorstr;
		return false;
	}

	return true;
}

bool Engine::Utils::test_and_error_bool(bool b, const QString& errorstr)
{
	if(!b) {
		sp_log(Log::Error, "Engine::Utils") << errorstr;
		return false;
	}

	return true;
}

bool Engine::Utils::create_element(GstElement** elem, const QString& elem_name)
{
	return create_element(elem, elem_name, QString());
}

bool Engine::Utils::create_element(GstElement** elem, const QString& elem_name, const QString& prefix)
{
	gchar* g_elem_name = g_strdup(elem_name.toLocal8Bit().data());

	QString error_msg;
	if(prefix.size() > 0)
	{
		QString prefixed = prefix + "_" + elem_name;
		gchar* g_prefixed = g_strdup(prefixed.toLocal8Bit().data());
		*elem = gst_element_factory_make(g_elem_name, g_prefixed);

		error_msg = QString("Engine: ") + prefixed + " creation failed";
	}

	else{
		*elem = gst_element_factory_make(g_elem_name, g_elem_name);
		error_msg = QString("Engine: ") + elem_name + " creation failed";
	}

	return test_and_error(*elem, error_msg);
}

MilliSeconds Engine::Utils::get_duration_ms(GstElement* element)
{
	if(!element){
		return -1;
	}

	NanoSeconds pos;
	bool success = gst_element_query_duration(element, GST_FORMAT_TIME, &pos);
	if(!success){
		return -1;
	}

	return GST_TIME_AS_MSECONDS(pos);
}

MilliSeconds Engine::Utils::get_position_ms(GstElement* element)
{
	if(!element){
		return -1;
	}

	NanoSeconds pos;
	bool success = gst_element_query_position(element, GST_FORMAT_TIME, &pos);
	if(!success){
		return -1;
	}

	return GST_TIME_AS_MSECONDS(pos);
}


MilliSeconds Engine::Utils::get_time_to_go(GstElement* element)
{
	if(!element){
		return -1;
	}

	MilliSeconds pos = get_position_ms(element);
	if(pos < 0){
		return get_duration_ms(element);
	}

	MilliSeconds dur = get_duration_ms(element);
	if(dur < 0){
		return -1;
	}

	if(dur < pos){
		return -1;
	}

	return dur - pos;
}

GstState Engine::Utils::get_state(GstElement* element)
{
	if(!element){
		return GST_STATE_NULL;
	}

	GstState state;
	gst_element_get_state(element, &state, nullptr, GST_MSECOND * 10);
	return state;
}

bool Engine::Utils::set_state(GstElement* element, GstState state)
{
	if(!element){
		return false;
	}

	GstStateChangeReturn ret = gst_element_set_state(element, state);
	return (ret != GST_STATE_CHANGE_FAILURE);
}


bool Engine::Utils::check_plugin_available(const gchar* str)
{
	GstRegistry* reg = gst_registry_get();
	GstPlugin* plugin = gst_registry_find_plugin(reg, str);

	bool success = (plugin != nullptr);
	gst_object_unref(plugin);

	return success;
}

bool Engine::Utils::check_pitch_available()
{
	return check_plugin_available("soundtouch");
}

bool Engine::Utils::check_lame_available()
{
	return check_plugin_available("lame");
}

bool Engine::Utils::create_ghost_pad(GstBin* bin, GstElement* e)
{
	GstPad* pad = gst_element_get_static_pad(e, "sink");
	if(!test_and_error(pad, "CreateGhostPad: Cannot get static pad")){
		return false;
	}

	GstPad* ghost_pad = gst_ghost_pad_new("sink", pad);
	if(!test_and_error(ghost_pad, "CreateGhostPad: Cannot create ghost pad")){
		return false;
	}

	gst_pad_set_active(ghost_pad, true);
	bool success = gst_element_add_pad(GST_ELEMENT(bin), ghost_pad);
	if(!test_and_error_bool(success, "CreateGhostPad: Cannot add ghost pad")){
		return false;
	}

	gst_object_unref(pad);
	return true;
}

bool Engine::Utils::create_bin(GstElement** bin, const QList<GstElement*>& elements, const QString& prefix)
{
	QString prefixed = prefix + "bin";
	gchar* g_name = g_strdup(prefixed.toLocal8Bit().data());
	*bin = gst_bin_new(g_name);
	if(!test_and_error(*bin, "Cannot create bin " + prefixed)){
		return false;
	}

	add_elements(GST_BIN(*bin), elements);
	bool success = link_elements(elements);
	if(!success) {
		unref_elements(elements);
		gst_object_unref(bin);
		*bin = nullptr;
		return false;
	}

	GstElement* e = elements.first();
	success = create_ghost_pad(GST_BIN(*bin), e);
	if(!success){
		unref_elements(elements);
		gst_object_unref(bin);
		*bin = nullptr;
		return false;
	}

	gst_object_ref(*bin);

	return true;
}

bool Engine::Utils::link_elements(const QList<GstElement*>& elements)
{
	bool success = true;
	for(int i=0; i<elements.size() - 1; i++)
	{
		GstElement* e1 = elements.at(i);
		GstElement* e2 = elements.at(i+1);

		gchar* n1 = gst_element_get_name(e1);
		gchar* n2 = gst_element_get_name(e2);

		sp_log(Log::Debug, "Engine::Utils") << "Try to link " << n1 << " with " << n2;

		bool b = gst_element_link(e1, e2);
		if(!b)
		{

			test_and_error_bool(b, QString("Cannot link element %1 with %2").arg(n1, n2));
			g_free(n1);
			g_free(n2);
			success = false;
			break;
		}
	}

	return success;
}

void Engine::Utils::add_elements(GstBin* bin, const QList<GstElement*>& elements)
{
	for(GstElement* e : elements){
		gst_bin_add(bin, e);
	}
}

void Engine::Utils::unref_elements(const QList<GstElement*>& elements)
{
	for(GstElement* e : elements){
		gst_object_unref(e);
	}
}

void Engine::Utils::config_queue(GstElement* queue, gulong max_time_ms)
{
	set_values(queue,
		"flush-on-eos", true,
		"silent", true);

	set_uint64_value(queue,  "max-size-time", max_time_ms * GST_MSECOND);
}

void Engine::Utils::config_sink(GstElement* sink)
{
	set_values(sink,
		"sync", true,
		"async", false);
}

void Engine::Utils::config_lame(GstElement* lame)
{
	set_values(lame,
		"perfect-timestamp", true,
		"cbr", true
	);

	set_int_value(lame, "bitrate", 128);
	set_int_value(lame, "target", 1);
	set_int_value(lame, "encoding-engine-quality", 2);
}


void Engine::Utils::set_passthrough(GstElement* e, bool b)
{
	if(e && GST_IS_BASE_TRANSFORM(e))
	{
		gst_base_transform_set_passthrough(GST_BASE_TRANSFORM(e), b);
		gst_base_transform_set_prefer_passthrough(GST_BASE_TRANSFORM(e), b);
	}
}


GValue Engine::Utils::get_int64(gint64 value)
{
	GValue ret = G_VALUE_INIT;
	g_value_init(&ret, G_TYPE_INT64);
	g_value_set_int64(&ret, value);
	return ret;
}

GValue Engine::Utils::get_uint64(guint64 value)
{
	GValue ret = G_VALUE_INIT;
	g_value_init(&ret, G_TYPE_INT64);
	g_value_set_int64(&ret, value);
	return ret;
}

GValue Engine::Utils::get_uint(guint value)
{
	GValue ret = G_VALUE_INIT;
	g_value_init(&ret, G_TYPE_UINT);
	g_value_set_uint(&ret, value);
	return ret;
}

GValue Engine::Utils::get_int(gint value)
{
	GValue ret = G_VALUE_INIT;
	g_value_init(&ret, G_TYPE_INT);
	g_value_set_int(&ret, value);
	return ret;
}

MilliSeconds Engine::Utils::get_update_interval()
{
	return 50;
}