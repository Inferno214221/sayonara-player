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

#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include "Utils/typedefs.h"

#include <type_traits>
#include <utility>
#include <memory>
#include <iostream>
#include <gst/gst.h>

template<typename T>
class QList;

namespace Engine
{


	/**
	 * @brief Utility functions
	 * @ingroup EngineHelper
	 */
	namespace Utils
	{

		using Elements=QList<GstElement*>;

		template<typename T>
		struct GObjectAutoFree
		{
			T* obj;

			GObjectAutoFree(T* obj) : obj(obj) {}
			~GObjectAutoFree() { g_free(obj); }
			T* data() const { return obj; }
		};

		/**
		 * @brief config_queue
		 * @param queue
		 * @param max_time_ms
		 */
		void config_queue(GstElement* queue, gulong max_time_ms=100);

		/**
		 * @brief config_sink
		 * @param sink
		 */
		void config_sink(GstElement* sink);

		/**
		 * @brief config_lame
		 * @param lame
		 */
		void config_lame(GstElement* lame);

		/**
		 * @brief tee_connect
		 * @param tee
		 * @param queue
		 * @param queue_name
		 * @return
		 */
		bool tee_connect(GstElement* tee, GstElement* queue, const QString& queue_name);

		/**
		 * @brief has_element
		 * @param bin
		 * @param element
		 * @return
		 */
		bool has_element(GstBin* bin, GstElement* element);

		/**
		 * @brief test_and_error
		 * @param element
		 * @param errorstr
		 * @return
		 */
		bool test_and_error(void* element, const QString& errorstr);

		/**
		 * @brief test_and_error_bool
		 * @param b
		 * @param errorstr
		 * @return
		 */
		bool test_and_error_bool(bool b, const QString& errorstr);

		/**
		 * @brief create_element
		 * @param elem
		 * @param elem_name
		 * @return
		 */
		bool create_element(GstElement** elem, const QString& elem_name);

		/**
		 * @brief create_element
		 * @param elem
		 * @param elem_name
		 * @param name
		 * @return
		 */
		bool create_element(GstElement** elem, const QString& elem_name, const QString& name);

		/**
		 * @brief set_passthrough
		 * @param e
		 * @param b
		 */
		void set_passthrough(GstElement* e, bool b);

		/**
		 * @brief get_int64
		 * @param value
		 * @return
		 */
		GValue get_int64(gint64 value);

		/**
		 * @brief get_uint64
		 * @param value
		 * @return
		 */
		GValue get_uint64(guint64 value);

		/**
		 * @brief get_uint
		 * @param value
		 * @return
		 */
		GValue get_uint(guint value);

		/**
		 * @brief get_int
		 * @param value
		 * @return
		 */
		GValue get_int(gint value);

		/**
		 * @brief get_update_interval
		 * @return
		 */
		MilliSeconds get_update_interval();

		template<typename T>

		/**
		 * @brief Class for compiler warnings
		 */
		struct Dont_Use_Integers_In_GObject_Set
		{
			Dont_Use_Integers_In_GObject_Set(T value)
			{
				std::string("There's a wrong value somewhere") + value;
			}
		};


		template<typename GlibObject, typename T>
		/**
		 * @brief set_value
		 * @param object
		 * @param key
		 * @param value
		 */
		void set_value(GlibObject* object, const gchar* key, T value, std::true_type)
		{
			(void) object;
			(void) key;
			(void) value;
			Dont_Use_Integers_In_GObject_Set<T>();
		}


		template<typename GlibObject, typename T>
		/**
		 * @brief set_value
		 * @param object
		 * @param key
		 * @param value
		 */
		void set_value(GlibObject* object, const gchar* key, T value, std::false_type)
		{
			g_object_set(G_OBJECT(object), key, value, nullptr);
		}


		template<typename GlibObject, typename T>
		/**
		 * @brief set_value
		 * @param object
		 * @param key
		 * @param value
		 */
		void set_value(GlibObject* object, const gchar* key, T value)
		{
			constexpr bool b = (std::is_integral<T>::value) && (sizeof(T) > sizeof(bool));
			set_value(object, key, value, std::integral_constant<bool, b>());
		}


		template<typename GlibObject, typename First>
		/**
		 * @brief set_values
		 * @param object
		 * @param key
		 * @param value
		 */
		void set_values(GlibObject* object, const gchar* key, First value)
		{
			set_value(object, key, value);
		}


		template<typename GlibObject, typename First, typename... Args>
		/**
		 * @brief set_values
		 * @param object
		 * @param key
		 * @param value
		 * @param args
		 */
		void set_values(GlibObject* object, const gchar* key, First value, Args... args)
		{
			set_value(object, key, value);
			set_values(object, std::forward<Args>(args)...);
		}


		template<typename GlibObject>
		/**
		 * @brief set_int64_value
		 * @param object
		 * @param key
		 * @param value
		 */
		void set_int64_value(GlibObject* object, const gchar* key, gint64 value)
		{
			GValue val = get_int64(value);
			g_object_set_property(G_OBJECT(object), key, &val);
		}


		template<typename GlibObject>
		/**
		 * @brief set_int_value
		 * @param object
		 * @param key
		 * @param value
		 */
		void set_int_value(GlibObject* object,const  gchar* key, gint value)
		{
			GValue val = get_int(value);
			g_object_set_property(G_OBJECT(object), key, &val);
		}



		template<typename GlibObject>
		/**
		 * @brief set_uint64_value
		 * @param object
		 * @param key
		 * @param value
		 */
		void set_uint64_value(GlibObject* object, const gchar* key, guint64 value)
		{
			GValue val = get_uint64(value);
			g_object_set_property(G_OBJECT(object), key, &val);
		}


		template<typename GlibObject>
		/**
		 * @brief set_uint_value
		 * @param object
		 * @param key
		 * @param value
		 */
		void set_uint_value(GlibObject* object, const gchar* key, guint value)
		{
			GValue val = get_uint(value);
			g_object_set_property(G_OBJECT(object), key, &val);
		}

		/**
		 * @brief get_duration_ms
		 * @param element
		 * @return
		 */
		MilliSeconds get_duration_ms(GstElement* element);

		/**
		 * @brief get_position_ms
		 * @param element
		 * @return
		 */
		MilliSeconds get_position_ms(GstElement* element);

		/**
		 * @brief get_time_to_go
		 * @param element
		 * @return
		 */
		MilliSeconds get_time_to_go(GstElement* element);

		/**
		 * @brief get_state
		 * @param element
		 * @return
		 */
		GstState get_state(GstElement* element);

		/**
		 * @brief set_state
		 * @param element
		 * @param state
		 * @return
		 */
		bool set_state(GstElement* element, GstState state);

		/**
		 * @brief check_plugin_available
		 * @param str
		 * @return
		 */
		bool check_plugin_available(const gchar* str);

		/**
		 * @brief check_lame_available
		 * @return
		 */
		bool check_lame_available();

		/**
		 * @brief check_pitch_available
		 * @return
		 */
		bool check_pitch_available();

		/**
		 * @brief create_bin
		 * @param bin
		 * @param elements
		 * @param prefix
		 * @return
		 */
		bool create_bin(GstElement** bin, const Elements& elements, const QString& prefix);

		/**
		 * @brief create_ghost_pad
		 * @param bin
		 * @param e
		 * @return
		 */
		bool create_ghost_pad(GstBin* bin, GstElement* e);

		/**
		 * @brief link_elements
		 * @param elements
		 * @return
		 */
		bool link_elements(const Elements& elements);

		void unlink_elements(const Elements& elements);

		/**
		 * @brief add_elements
		 * @param bin
		 * @param elements
		 */
		bool add_elements(GstBin* bin, const Elements& elements);

		void remove_elements(GstBin* bin, const Elements& elements);

		/**
		 * @brief unref_elements
		 * @param elements
		 */
		void unref_elements(const Elements& elements);
	}
}

#endif // ENGINE_UTILS_H
