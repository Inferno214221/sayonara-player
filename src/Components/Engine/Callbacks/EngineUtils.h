#ifndef ENGINEUTILS_H
#define ENGINEUTILS_H

#include <gst/gst.h>
#include "Utils/typedefs.h"

namespace Engine
{
	namespace Utils
	{
		void config_queue(GstElement* queue);
		void config_sink(GstElement* sink);
		void config_lame(GstElement* lame);

		bool tee_connect(GstElement* tee, GstElement* queue, const QString& queue_name);
		bool has_element(GstBin* bin, GstElement* element);
		bool test_and_error(void* element, const QString& errorstr);
		bool test_and_error_bool(bool b, const QString& errorstr);
		bool create_element(GstElement** elem, const QString& elem_name);
		bool create_element(GstElement** elem, const QString& elem_name, const QString& name);

		MilliSeconds get_duration_ms(GstElement* element);
		MilliSeconds get_position_ms(GstElement* element);
		MilliSeconds get_time_to_go(GstElement* element);

		GstState get_state(GstElement* element);
		bool set_state(GstElement* element, GstState state);
		bool check_lame_available();

		bool create_bin(GstElement** bin, const QList<GstElement*>& elements, const QString& prefix);

		bool create_ghost_pad(GstBin* bin, GstElement* e);
		bool link_elements(const QList<GstElement*>& elements);
		void add_elements(GstBin* bin, const QList<GstElement*>& elements);
		void unref_elements(const QList<GstElement*>& elements);

		void print_all_elements(GstBin* bin);

	}
}

#endif // ENGINEUTILS_H
