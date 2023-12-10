/* EngineUtils.h */

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

#ifndef SAYONARA_PLAYER_ENGINE_UTILS_H
#define SAYONARA_PLAYER_ENGINE_UTILS_H

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
	namespace Utils
	{
		using Elements = QList<GstElement*>;

		template<typename T>
		struct GObjectAutoFree
		{
			T* obj = nullptr;

			GObjectAutoFree(T* obj) :
				obj(obj) {}

			~GObjectAutoFree()
			{
				if(obj)
				{
					g_free(obj);
				}

				obj = nullptr;
			}

			T* data() const { return obj; }
		};

		class AutoUnref
		{
			public:
				AutoUnref(gpointer element) : // NOLINT(google-explicit-constructor)
					m_element {element} {}

				~AutoUnref()
				{
					gst_object_unref(m_element);
				}

				gpointer operator*() const { return m_element; }

			private:
				gpointer m_element;
		};

		using GStringAutoFree = GObjectAutoFree<gchar>;

		void configureQueue(GstElement* queue, guint64 max_time_ms = 100);

		void configureSink(GstElement* sink);

		void configureLame(GstElement* lame);

		bool connectTee(GstElement* tee, GstElement* queue, const QString& queue_name);

		bool hasElement(GstBin* bin, GstElement* element);

		bool testAndError(void* element, const QString& errorstr);

		bool testAndErrorBool(bool b, const QString& errorstr);

		bool createElement(GstElement** elem, const QString& elem_name);

		bool createElement(GstElement** elem, const QString& elem_name, const QString& name);

		void setPassthrough(GstElement* e, bool b);

		GValue getInt64(gint64 value);

		GValue getUint64(guint64 value);

		GValue getUint(guint value);

		GValue getInt(gint value);

		MilliSeconds getUpdateInterval();

		template<typename T>
		struct Dont_Use_Integers_In_GObject_Set
		{
			Dont_Use_Integers_In_GObject_Set(T value)
			{
				std::string("There's a wrong value somewhere") + value;
			}
		};

		template<typename GlibObject, typename T>
		void setValue(GlibObject* object, const gchar* key, T value, std::true_type)
		{
			(void) object;
			(void) key;
			(void) value;
			Dont_Use_Integers_In_GObject_Set<T>();
		}

		template<typename GlibObject, typename T>
		void setValue(GlibObject* object, const gchar* key, T value, std::false_type)
		{
			g_object_set(G_OBJECT(object), key, value, nullptr);
		}

		template<typename GlibObject, typename T>
		void setValue(GlibObject* object, const gchar* key, T value)
		{
			constexpr bool b = (std::is_integral<T>::value) && (sizeof(T) > sizeof(bool));
			setValue(object, key, value, std::integral_constant<bool, b>());
		}

		template<typename GlibObject, typename First>
		void setValues(GlibObject* object, const gchar* key, First value)
		{
			setValue(object, key, value);
		}

		template<typename GlibObject, typename First, typename... Args>
		void setValues(GlibObject* object, const gchar* key, First value, Args... args)
		{
			setValue(object, key, value);
			setValues(object, std::forward<Args>(args)...);
		}

		template<typename GlibObject>
		void setInt64Value(GlibObject* object, const gchar* key, gint64 value)
		{
			GValue val = getInt64(value);
			g_object_set_property(G_OBJECT(object), key, &val);
		}

		template<typename GlibObject>
		void setIntValue(GlibObject* object, const gchar* key, gint value)
		{
			GValue val = getInt(value);
			g_object_set_property(G_OBJECT(object), key, &val);
		}

		template<typename GlibObject>
		void setUint64Value(GlibObject* object, const gchar* key, guint64 value)
		{
			GValue val = getUint64(value);
			g_object_set_property(G_OBJECT(object), key, &val);
		}

		template<typename GlibObject>
		void setUintValue(GlibObject* object, const gchar* key, guint value)
		{
			GValue val = getUint(value);
			g_object_set_property(G_OBJECT(object), key, &val);
		}

		MilliSeconds getDurationMs(GstElement* element);

		MilliSeconds getPositionMs(GstElement* element);

		MilliSeconds getTimeToGo(GstElement* element);

		GstState getState(GstElement* element);

		bool setState(GstElement* element, GstState state);

		bool isPluginAvailable(const gchar* str);

		bool isLameAvailable();

		bool isPitchAvailable();

		bool createBin(GstElement** bin, const Elements& elements, const QString& prefix);

		bool createGhostPad(GstBin* bin, GstElement* e);

		bool linkElements(const Elements& elements);

		void unlinkElements(const Elements& elements);

		bool addElements(GstBin* bin, const Elements& elements);

		void removeElements(GstBin* bin, const Elements& elements);

		void unrefElements(const Elements& elements);
	}
}

#endif // SAYONARA_PLAYER_ENGINE_UTILS_H
