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

namespace
{
	struct TeeProbeData
	{
		GstState state;
		GstElement* element;
	};

	Engine::Utils::GStringAutoFree qstringToString(const QString& str)
	{
		return Engine::Utils::GStringAutoFree {g_strdup(str.toLocal8Bit().constData())};
	}

	GstPadProbeReturn teeProbleBlocked(GstPad* pad, GstPadProbeInfo* info, gpointer p)
	{
		auto data = std::shared_ptr<TeeProbeData> {static_cast<TeeProbeData*>(p)};
		auto* queue = data->element;

		if(!Engine::Utils::testAndError(queue, "Connect to tee: Element is not GstElement"))
		{
			return GST_PAD_PROBE_DROP;
		}

		auto queuePad = Engine::Utils::AutoUnref {gst_element_get_static_pad(queue, "sink")};
		if(!Engine::Utils::testAndError(*queuePad, "Connect to tee: No valid pad from GstElement"))
		{
			return GST_PAD_PROBE_DROP;
		}

		if(const auto padLinkReturn = gst_pad_link(pad, *queuePad); (padLinkReturn != GST_PAD_LINK_OK))
		{
			spLog(Log::Warning, "AbstractPipeline") << "Could not dynamically connect tee";
		}

		gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID (info));
		gst_element_set_state(queue, data->state);

		return GST_PAD_PROBE_DROP;
	}
}

bool Engine::Utils::connectTee(GstElement* tee, GstElement* queue, const QString& queueName)
{
	if(!testAndError(tee, "tee connect: tee is null") ||
	   !testAndError(queue, "tee connect: queue is null"))
	{
		return false;
	}

	auto* teeSrcPadTemplate = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(tee), "src_%u");
	if(!testAndError(teeSrcPadTemplate, "Engine: teeSrcPadTemplate is nullptr"))
	{
		return false;
	}

	auto teeQueuePad = AutoUnref {gst_element_request_pad(tee, teeSrcPadTemplate, nullptr, nullptr)};
	const auto errCannotRequestPad = QString("Engine: Tee-%1 pad is nullptr").arg(queueName);
	if(!testAndError(*teeQueuePad, errCannotRequestPad))
	{
		return false;
	}

	const auto state = Engine::Utils::getState(tee);
	if(state == GST_STATE_PLAYING || state == GST_STATE_PAUSED)
	{
		auto* data = new TeeProbeData();
		data->state = state;
		data->element = queue;

		gst_pad_add_probe(*teeQueuePad, GST_PAD_PROBE_TYPE_IDLE, teeProbleBlocked, data, nullptr);

		return true;
	}

	auto queuePad = AutoUnref(gst_element_get_static_pad(queue, "sink"));
	const auto errCannotGetStaticPad = QString("Engine: %1 pad is nullptr").arg(queueName);
	if(!testAndError(*queuePad, errCannotGetStaticPad))
	{
		return false;
	}

	const auto padLinkReturn = gst_pad_link(*teeQueuePad, *queuePad);
	const auto errCannotLink = QString("Engine: Cannot link tee with %1").arg(queueName);
	if(!testAndErrorBool((padLinkReturn == GST_PAD_LINK_OK), errCannotLink))
	{
		return false;
	}

	setState(queue, getState(tee));

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

	const auto elementName = getElementName(element);
	const auto binName = getObjectName(GST_OBJECT(bin));

	if(elementName.data() == nullptr || binName.data() == nullptr)
	{
		return false;
	}

	if(elementName == binName)
	{
		return true;
	}

	auto* parent = gst_object_get_parent(GST_OBJECT(element));
	while(parent != nullptr)
	{
		const auto parentName = getObjectName(parent);
		if(binName == parentName)
		{
			return true;
		}

		auto oldParent = AutoUnref {parent};
		parent = gst_object_get_parent(*oldParent);
	}

	return false;
}

bool Engine::Utils::testAndError(void* element, const QString& errorstr)
{
	if(!element)
	{
		spLog(Log::Error, "Engine::Utils") << errorstr;
	}

	return (element != nullptr);
}

bool Engine::Utils::testAndErrorBool(const bool b, const QString& errorstr)
{
	if(!b)
	{
		spLog(Log::Error, "Engine::Utils") << errorstr;
	}

	return b;
}

bool Engine::Utils::createElement(GstElement** elem, const QString& elementName)
{
	return createElement(elem, elementName, {});
}

bool Engine::Utils::createElement(GstElement** elem, const QString& elementType, const QString& prefix)
{
	const auto gElementType = qstringToString(elementType);
	const auto gElementName = (prefix.isEmpty())
	                          ? qstringToString(elementType)
	                          : qstringToString(QString("%1_%2").arg(prefix).arg(elementType));

	*elem = gst_element_factory_make(*gElementType, *gElementName);

	const auto errorMessage = QString("Engine: Cannot create element '%1' of type '%2'")
		.arg(prefix)
		.arg(elementType);

	setState(*elem, GST_STATE_NULL);

	return testAndError(*elem, errorMessage);
}

MilliSeconds Engine::Utils::getDurationMs(GstElement* element)
{
	if(!element)
	{
		return -1;
	}

	NanoSeconds duration {0};

	const auto success = gst_element_query_duration(element, GST_FORMAT_TIME, &duration);
	return success ? GST_TIME_AS_MSECONDS(duration) : -1;
}

MilliSeconds Engine::Utils::getPositionMs(GstElement* element)
{
	if(!element)
	{
		return -1;
	}

	NanoSeconds pos {0};
	const auto success = gst_element_query_position(element, GST_FORMAT_TIME, &pos);
	return success ? GST_TIME_AS_MSECONDS(pos) : -1;
}

MilliSeconds Engine::Utils::getTimeToGo(GstElement* element)
{
	if(!element)
	{
		return -1;
	}

	const auto position = getPositionMs(element);
	if(position < 0)
	{
		return getDurationMs(element);
	}

	const auto duration = getDurationMs(element);
	if(duration < 0)
	{
		return -1;
	}

	return duration >= position
	       ? duration - position
	       : -1;

}

GstState Engine::Utils::getState(GstElement* element)
{
	if(!element)
	{
		return GST_STATE_NULL;
	}

	GstState state {GST_STATE_NULL};
	gst_element_get_state(element, &state, nullptr, 10 * GST_MSECOND);

	return state;
}

bool Engine::Utils::setState(GstElement* element, GstState state)
{
	if(!element)
	{
		return false;
	}

	const auto ret = gst_element_set_state(element, state);

	return (ret != GST_STATE_CHANGE_FAILURE);
}

bool Engine::Utils::isPluginAvailable(const gchar* str)
{
	auto* reg = gst_registry_get();
	auto plugin = AutoUnref {gst_registry_find_plugin(reg, str)};

	return (*plugin != nullptr);
}

bool Engine::Utils::isPitchAvailable() { return isPluginAvailable("soundtouch"); }

bool Engine::Utils::isLameAvailable() { return isPluginAvailable("lame"); }

bool Engine::Utils::createGhostPad(GstBin* bin, GstElement* e)
{
	auto* pad = gst_element_get_static_pad(e, "sink");
	if(!testAndError(pad, "CreateGhostPad: Cannot get static pad"))
	{
		return false;
	}

	auto* ghostPad = gst_ghost_pad_new("sink", pad);
	if(!testAndError(ghostPad, "CreateGhostPad: Cannot create ghost pad"))
	{
		return false;
	}

	gst_pad_set_active(ghostPad, true);

	const auto padAdded = gst_element_add_pad(GST_ELEMENT(bin), ghostPad);
	if(!testAndErrorBool(padAdded, "CreateGhostPad: Cannot add ghost pad"))
	{
		return false;
	}

	gst_object_unref(pad);

	return true;
}

bool Engine::Utils::createBin(GstElement** bin, const QList<GstElement*>& elements, const QString& prefix)
{
	const auto prefixed = QString("%1bin").arg(prefix);
	auto gName = qstringToString(prefixed);
	*bin = gst_bin_new(*gName);
	if(!testAndError(*bin, "Cannot create bin " + prefixed))
	{
		return false;
	}

	addElements(GST_BIN(*bin), elements);

	if(const auto linked = linkElements(elements); !linked)
	{
		unrefElements(elements);
		gst_object_unref(bin);
		*bin = nullptr;
		return false;
	}

	auto* element = elements.first();
	if(const auto ghostPadCreated = createGhostPad(GST_BIN(*bin), element); !ghostPadCreated)
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
	auto success = true;
	for(int i = 0; i < elements.size() - 1; i++)
	{
		auto* e1 = elements.at(i);
		auto* e2 = elements.at(i + 1);

		if(!e2)
		{
			break;
		}

		auto name1 = getElementName(e1);
		auto name2 = getElementName(e2);

		spLog(Log::Debug, "Engine::Utils") << "Try to link " << name1 << " with " << name2;

		if(const auto linked = gst_element_link(e1, e2); !linked)
		{
			testAndErrorBool(linked, QString("Cannot link element %1 with %2").arg(name1, name2));
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
		auto* e1 = elements.at(i);
		auto* e2 = elements.at(i + 1);

		if(!e2)
		{
			break;
		}

		auto name1 = getElementName(e1);
		auto name2 = getElementName(e2);

		spLog(Log::Debug, "Engine::Utils") << "Try to unlink " << name1 << " with " << name2;

		gst_element_unlink(e1, e2);
	}
}

bool Engine::Utils::addElements(GstBin* bin, const QList<GstElement*>& elements)
{
	for(auto* element: elements) // NOLINT(readability-use-anyofallof)
	{
		if(!element || hasElement(bin, element))
		{
			continue;
		}

		if(const auto added = gst_bin_add(bin, element); !added)
		{
			return false;
		}
	}

	return true;
}

void Engine::Utils::removeElements(GstBin* bin, const Engine::Utils::Elements& elements)
{
	for(auto* e: elements)
	{
		if(!e || !hasElement(bin, e))
		{
			continue;
		}

		if(const auto removed = gst_bin_remove(bin, e); !removed)
		{
			spLog(Log::Warning, "Engine::Utils") << "Could not remove element " << getElementName(e);
		}
	}
}

void Engine::Utils::unrefElements(const QList<GstElement*>& elements)
{
	for(auto* e: elements)
	{
		gst_object_unref(e);
	}
}

void Engine::Utils::configureQueue(GstElement* queue, guint64 maxTimeMs)
{
	setValues(queue,
	          "flush-on-eos", true,
	          "silent", true);

	setUint64Value(queue, "max-size-time", guint64(maxTimeMs * GST_MSECOND));
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

	setIntValue(lame, "bitrate", 128); // NOLINT(readability-magic-numbers)
	setIntValue(lame, "target", 1);
	setIntValue(lame, "encoding-engine-quality", 2);
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

QString Engine::Utils::getElementName(const GstElement* element)
{
	auto name = GStringAutoFree {gst_element_get_name(element)};
	return QString {*name};
}

QString Engine::Utils::getObjectName(GstObject* object)
{
	auto name = GStringAutoFree {gst_object_get_name(object)};
	return QString {*name};
}

Engine::Utils::AutoUnref<GstPad> Engine::Utils::getStaticPad(GstElement* element, Engine::Utils::PadType padType)
{
	const auto* padDescription = (padType == PadType::src) ? "src" : "sink";
	auto* pad = gst_element_get_static_pad(element, padDescription);
	return Engine::Utils::AutoUnref {pad};
}

