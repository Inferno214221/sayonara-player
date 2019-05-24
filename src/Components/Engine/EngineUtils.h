/* EngineUtils.h */

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

#ifndef ENGINEUTILS_H
#define ENGINEUTILS_H

#include <gst/gst.h>
#include "Utils/typedefs.h"
#include <type_traits>
#include <utility>
#include <memory>
#include <iostream>

namespace EngineUtils
{
	void config_queue(GstElement* queue, gulong max_time_ms=100);
	void config_sink(GstElement* sink);
	void config_lame(GstElement* lame);

	bool tee_connect(GstElement* tee, GstElement* queue, const QString& queue_name);
	bool has_element(GstBin* bin, GstElement* element);
	bool test_and_error(void* element, const QString& errorstr);
	bool test_and_error_bool(bool b, const QString& errorstr);
	bool create_element(GstElement** elem, const QString& elem_name);
	bool create_element(GstElement** elem, const QString& elem_name, const QString& name);

	void set_passthrough(GstElement* e, bool b);

	GValue get_int64(gint64 value);
	GValue get_uint64(guint64 value);
	GValue get_uint(guint value);
	GValue get_int(gint value);

	template<typename T>
	struct Dont_Use_Integers_In_GObject_Set
	{
		Dont_Use_Integers_In_GObject_Set(T value)
		{
			std::string("There's a wrong value somewhere") + value;
		}
	};

	template<typename GlibObject, typename T>
	void set_value(GlibObject* object, const gchar* key, T value, std::true_type)
	{
		(void) object;
		(void) key;
		(void) value;
		Dont_Use_Integers_In_GObject_Set<T>();
	}

	template<typename GlibObject, typename T>
	void set_value(GlibObject* object, const gchar* key, T value, std::false_type)
	{
		g_object_set(G_OBJECT(object), key, value, nullptr);
	}

	template<typename GlibObject, typename T>
	void set_value(GlibObject* object, const gchar* key, T value)
	{
		constexpr bool b = (std::is_integral<T>::value) && (sizeof(T) > sizeof(bool));
		set_value(object, key, value, std::integral_constant<bool, b>());
	}

	template<typename GlibObject, typename First>
	void set_values(GlibObject* object, const gchar* key, First value)
	{
		set_value(object, key, value);
	}

	template<typename GlibObject, typename First, typename... Args>
	void set_values(GlibObject* object, const gchar* key, First value, Args... args)
	{
		set_value(object, key, value);
		set_values(object, std::forward<Args>(args)...);
	}

	template<typename GlibObject>
	void set_int64_value(GlibObject* object, const gchar* key, gint64 value)
	{
		GValue val = get_int64(value);
		g_object_set_property(G_OBJECT(object), key, &val);
	}

	template<typename GlibObject>
	void set_int_value(GlibObject* object,const  gchar* key, gint value)
	{
		GValue val = get_int(value);
		g_object_set_property(G_OBJECT(object), key, &val);
	}

	template<typename GlibObject>
	void set_uint64_value(GlibObject* object, const gchar* key, guint64 value)
	{
		GValue val = get_uint64(value);
		g_object_set_property(G_OBJECT(object), key, &val);
	}
	template<typename GlibObject>
	void set_uint_value(GlibObject* object, const gchar* key, guint value)
	{
		GValue val = get_uint(value);
		g_object_set_property(G_OBJECT(object), key, &val);
	}

	MilliSeconds get_duration_ms(GstElement* element);
	MilliSeconds get_position_ms(GstElement* element);
	MilliSeconds get_time_to_go(GstElement* element);

	GstState get_state(GstElement* element);
	bool set_state(GstElement* element, GstState state);

	bool check_plugin_available(const gchar* str);
	bool check_lame_available();
	bool check_pitch_available();

	bool create_bin(GstElement** bin, const QList<GstElement*>& elements, const QString& prefix);

	bool create_ghost_pad(GstBin* bin, GstElement* e);
	bool link_elements(const QList<GstElement*>& elements);
	void add_elements(GstBin* bin, const QList<GstElement*>& elements);
	void unref_elements(const QList<GstElement*>& elements);

}

#endif // ENGINEUTILS_H
