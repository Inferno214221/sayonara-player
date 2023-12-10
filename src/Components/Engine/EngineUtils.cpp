/* EngineUtils.cpp */

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

#include "EngineUtils.h"
#include "Utils/Logger/Logger.h"

#include <QString>
#include <QStringList>

#include <gst/base/gstbasetransform.h>

struct TeeProbeData
{
	GstState state;
	GstElement* element;
};

static GstPadProbeReturn
teeProbleBlocked(GstPad* pad, GstPadProbeInfo* info, gpointer p)
{
	TeeProbeData* data = static_cast<TeeProbeData*>(p);
	GstElement* queue = data->element;

	if(!Engine::Utils::testAndError(queue, "Connect to tee: Element is not GstElement"))
	{
		delete data;
		data = nullptr;
		return GST_PAD_PROBE_DROP;
	}

	GstPad* queue_pad = gst_element_get_static_pad(queue, "sink");
	if(!Engine::Utils::testAndError(queue_pad, "Connect to tee: No valid pad from GstElement"))
	{
		delete data;
		data = nullptr;
		return GST_PAD_PROBE_DROP;
	}

	GstPadLinkReturn s = gst_pad_link(pad, queue_pad);
	if(s != GST_PAD_LINK_OK)
	{
		spLog(Log::Warning, "AbstractPipeline") << "Could not dynamically connect tee";
	}

	gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID (info));
	gst_element_set_state(queue, data->state);

	delete data;
	data = nullptr;
	return GST_PAD_PROBE_DROP;
}

bool Engine::Utils::connectTee(GstElement* tee, GstElement* queue, const QString& queue_name)
{
	if(!testAndError(tee, "tee connect: tee is null"))
	{
		return false;
	}

	if(!testAndError(queue, "tee connect: queue is null"))
	{
		return false;
	}

	QString error_1 = QString("Engine: Tee-") + queue_name + " pad is nullptr";
	QString error_2 = QString("Engine: ") + queue_name + " pad is nullptr";
	QString error_3 = QString("Engine: Cannot link tee with ") + queue_name;

	GstPadTemplate* tee_src_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(tee), "src_%u");
	if(!testAndError(tee_src_pad_template, "Engine: _tee_src_pad_template is nullptr"))
	{
		return false;
	}

	GstPad* tee_queue_pad = gst_element_request_pad(tee, tee_src_pad_template, nullptr, nullptr);
	if(!testAndError(tee_queue_pad, error_1))
	{
		return false;
	}

	GstState state = Engine::Utils::getState(tee);

	if(state == GST_STATE_PLAYING || state == GST_STATE_PAUSED)
	{
		TeeProbeData* data = new TeeProbeData();
		data->state = state;
		data->element = queue;

		gulong id = gst_pad_add_probe(tee_queue_pad,
		                              GST_PAD_PROBE_TYPE_IDLE,
		                              teeProbleBlocked,
		                              data,
		                              nullptr);

		Q_UNUSED(id)

		return true;
	}

	GstPad* queue_pad = gst_element_get_static_pad(queue, "sink");
	if(!testAndError(queue_pad, error_2))
	{
		return false;
	}

	GstPadLinkReturn s = gst_pad_link(tee_queue_pad, queue_pad);
	if(!testAndErrorBool((s == GST_PAD_LINK_OK), error_3))
	{
		return false;
	}

	setState(queue, getState(tee));

	gst_object_unref(tee_queue_pad);
	gst_object_unref(queue_pad);
	return true;
}

bool Engine::Utils::hasElement(GstBin* bin, GstElement* element)
{
	if(!bin || !element)
	{
		return true;
	}

	if(!GST_OBJECT(element) || !GST_OBJECT(bin))
	{
		return false;
	}

	Engine::Utils::GStringAutoFree element_name(gst_element_get_name(element));
	Engine::Utils::GStringAutoFree bin_name(gst_object_get_name(GST_OBJECT(bin)));

	if(element_name.data() == nullptr || bin_name.data() == nullptr)
	{
		return false;
	}

	if(strncmp(element_name.data(), bin_name.data(), 40) == 0)
	{
		return true;
	}

	GstObject* parent = gst_object_get_parent(GST_OBJECT(element));

	while(parent != nullptr)
	{
		Engine::Utils::GStringAutoFree parent_name(gst_object_get_name(parent));

		if(strncmp(bin_name.data(), parent_name.data(), 50) == 0)
		{
			return true;
		}

		auto* old_parent = parent;
		parent = gst_object_get_parent(old_parent);
		gst_object_unref(old_parent);
	}

	return false;
}

bool Engine::Utils::testAndError(void* element, const QString& errorstr)
{
	if(!element)
	{
		spLog(Log::Error, "Engine::Utils") << errorstr;
		return false;
	}

	return true;
}

bool Engine::Utils::testAndErrorBool(bool b, const QString& errorstr)
{
	if(!b)
	{
		spLog(Log::Error, "Engine::Utils") << errorstr;
		return false;
	}

	return true;
}

bool Engine::Utils::createElement(GstElement** elem, const QString& elem_name)
{
	return createElement(elem, elem_name, QString());
}

bool Engine::Utils::createElement(GstElement** elem, const QString& elem_name, const QString& prefix)
{
	gchar* g_elem_name = g_strdup(elem_name.toLocal8Bit().data());

	QString error_msg;
	if(prefix.size() > 0)
	{
		QString prefixed = prefix + "_" + elem_name;
		gchar* g_prefixed = g_strdup(prefixed.toLocal8Bit().data());
		*elem = gst_element_factory_make(g_elem_name, g_prefixed);

		error_msg = QString("Engine: ") + prefixed + " creation failed";
		g_free(g_prefixed);
	}

	else
	{
		*elem = gst_element_factory_make(g_elem_name, g_elem_name);
		error_msg = QString("Engine: ") + elem_name + " creation failed";
	}

	setState(*elem, GST_STATE_NULL);

	g_free(g_elem_name);

	return testAndError(*elem, error_msg);
}

MilliSeconds Engine::Utils::getDurationMs(GstElement* element)
{
	if(!element)
	{
		return -1;
	}

	NanoSeconds pos;
	bool success = gst_element_query_duration(element, GST_FORMAT_TIME, &pos);
	if(!success)
	{
		return -1;
	}

	return GST_TIME_AS_MSECONDS(pos);
}

MilliSeconds Engine::Utils::getPositionMs(GstElement* element)
{
	if(!element)
	{
		return -1;
	}

	NanoSeconds pos;
	bool success = gst_element_query_position(element, GST_FORMAT_TIME, &pos);
	if(!success)
	{
		return -1;
	}

	return GST_TIME_AS_MSECONDS(pos);
}

MilliSeconds Engine::Utils::getTimeToGo(GstElement* element)
{
	if(!element)
	{
		return -1;
	}

	MilliSeconds pos = getPositionMs(element);
	if(pos < 0)
	{
		return getDurationMs(element);
	}

	MilliSeconds dur = getDurationMs(element);
	if(dur < 0)
	{
		return -1;
	}

	if(dur < pos)
	{
		return -1;
	}

	return dur - pos;
}

GstState Engine::Utils::getState(GstElement* element)
{
	if(!element)
	{
		return GST_STATE_NULL;
	}

	GstState state;
	gst_element_get_state(element, &state, nullptr, GST_MSECOND * 10);
	return state;
}

bool Engine::Utils::setState(GstElement* element, GstState state)
{
	if(!element)
	{
		return false;
	}

	GstStateChangeReturn ret = gst_element_set_state(element, state);
	return (ret != GST_STATE_CHANGE_FAILURE);
}

bool Engine::Utils::isPluginAvailable(const gchar* str)
{
	GstRegistry* reg = gst_registry_get();
	GstPlugin* plugin = gst_registry_find_plugin(reg, str);

	bool success = (plugin != nullptr);
	gst_object_unref(plugin);

	return success;
}

bool Engine::Utils::isPitchAvailable()
{
	return isPluginAvailable("soundtouch");
}

bool Engine::Utils::isLameAvailable()
{
	return isPluginAvailable("lame");
}

bool Engine::Utils::createGhostPad(GstBin* bin, GstElement* e)
{
	GstPad* pad = gst_element_get_static_pad(e, "sink");
	if(!testAndError(pad, "CreateGhostPad: Cannot get static pad"))
	{
		return false;
	}

	GstPad* ghost_pad = gst_ghost_pad_new("sink", pad);
	if(!testAndError(ghost_pad, "CreateGhostPad: Cannot create ghost pad"))
	{
		return false;
	}

	gst_pad_set_active(ghost_pad, true);
	bool success = gst_element_add_pad(GST_ELEMENT(bin), ghost_pad);
	if(!testAndErrorBool(success, "CreateGhostPad: Cannot add ghost pad"))
	{
		return false;
	}

	gst_object_unref(pad);
	return true;
}

bool Engine::Utils::createBin(GstElement** bin, const QList<GstElement*>& elements, const QString& prefix)
{
	QString prefixed = prefix + "bin";
	gchar* g_name = g_strdup(prefixed.toLocal8Bit().data());
	*bin = gst_bin_new(g_name);
	if(!testAndError(*bin, "Cannot create bin " + prefixed))
	{
		return false;
	}

	addElements(GST_BIN(*bin), elements);
	bool success = linkElements(elements);
	if(!success)
	{
		unrefElements(elements);
		gst_object_unref(bin);
		*bin = nullptr;
		return false;
	}

	GstElement* e = elements.first();
	success = createGhostPad(GST_BIN(*bin), e);
	if(!success)
	{
		unrefElements(elements);
		gst_object_unref(bin);
		*bin = nullptr;
		return false;
	}

	gst_object_ref(*bin);

	return true;
}

bool Engine::Utils::linkElements(const QList<GstElement*>& elements)
{
	bool success = true;
	for(int i = 0; i < elements.size() - 1; i++)
	{
		GstElement* e1 = elements.at(i);
		GstElement* e2 = elements.at(i + 1);

		if(!e2)
		{
			break;
		}

		gchar* n1 = gst_element_get_name(e1);
		gchar* n2 = gst_element_get_name(e2);

		spLog(Log::Debug, "Engine::Utils") << "Try to link " << n1 << " with " << n2;

		bool b = gst_element_link(e1, e2);
		if(!b)
		{
			testAndErrorBool(b, QString("Cannot link element %1 with %2").arg(n1, n2));
			g_free(n1);
			g_free(n2);
			success = false;
			break;
		}
	}

	return success;
}

void Engine::Utils::unlinkElements(const Engine::Utils::Elements& elements)
{
	for(int i = 0; i < elements.size() - 1; i++)
	{
		GstElement* e1 = elements.at(i);
		GstElement* e2 = elements.at(i + 1);

		if(!e2)
		{
			break;
		}

		gchar* n1 = gst_element_get_name(e1);
		gchar* n2 = gst_element_get_name(e2);

		spLog(Log::Debug, "Engine::Utils") << "Try to unlink " << n1 << " with " << n2;

		gst_element_unlink(e1, e2);
		g_free(n1);
		g_free(n2);
	}
}

bool Engine::Utils::addElements(GstBin* bin, const QList<GstElement*>& elements)
{
	bool b = true;
	for(GstElement* e: elements)
	{
		if(!e || hasElement(bin, e))
		{
			continue;
		}

		b = (b && gst_bin_add(bin, e));
		if(!b)
		{
			break;
		}
	}

	return b;
}

void Engine::Utils::removeElements(GstBin* bin, const Engine::Utils::Elements& elements)
{
	for(GstElement* e: elements)
	{
		if(!e || !hasElement(bin, e))
		{
			continue;
		}

		bool success = gst_bin_remove(bin, e);
		if(!success)
		{
			gchar* name = gst_element_get_name(e);
			spLog(Log::Warning, "Engine::Utils") << "Could not remove element " << name;
			g_free(name);
		}
	}
}

void Engine::Utils::unrefElements(const QList<GstElement*>& elements)
{
	for(GstElement* e: elements)
	{
		gst_object_unref(e);
	}
}

void Engine::Utils::configureQueue(GstElement* queue, guint64 max_time_ms)
{
	setValues(queue,
	          "flush-on-eos", true,
	          "silent", true);

	setUint64Value(queue, "max-size-time", guint64(max_time_ms * GST_MSECOND));
}

void Engine::Utils::configureSink(GstElement* sink)
{
	setValues(sink,
	          "sync", true,
	          "async", false);
}

void Engine::Utils::configureLame(GstElement* lame)
{
	setValues(lame,
	          "perfect-timestamp", true,
	          "cbr", true
	);

	setIntValue(lame, "bitrate", 128);
	setIntValue(lame, "target", 1);
	setIntValue(lame, "encoding-engine-quality", 2);
}

void Engine::Utils::setPassthrough(GstElement* e, bool b)
{
	if(e && GST_IS_BASE_TRANSFORM(e))
	{
		gst_base_transform_set_passthrough(GST_BASE_TRANSFORM(e), b);
		gst_base_transform_set_prefer_passthrough(GST_BASE_TRANSFORM(e), b);
	}
}

GValue Engine::Utils::getInt64(gint64 value)
{
	GValue ret = G_VALUE_INIT;
	g_value_init(&ret, G_TYPE_INT64);
	g_value_set_int64(&ret, value);
	return ret;
}

GValue Engine::Utils::getUint64(guint64 value)
{
	GValue ret = G_VALUE_INIT;
	g_value_init(&ret, G_TYPE_UINT64);
	g_value_set_uint64(&ret, value);
	return ret;
}

GValue Engine::Utils::getUint(guint value)
{
	GValue ret = G_VALUE_INIT;
	g_value_init(&ret, G_TYPE_UINT);
	g_value_set_uint(&ret, value);
	return ret;
}

GValue Engine::Utils::getInt(gint value)
{
	GValue ret = G_VALUE_INIT;
	g_value_init(&ret, G_TYPE_INT);
	g_value_set_int(&ret, value);
	return ret;
}

MilliSeconds Engine::Utils::getUpdateInterval()
{
	return 50;
}
