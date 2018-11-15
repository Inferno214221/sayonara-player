/* EngineCallbacks.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "EngineCallbacks.h"
#include "Components/Engine/Playback/PlaybackEngine.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/globals.h"
#include "Utils/Logger/Logger.h"

#include <QList>
#include <QImage>
#include <QRegExp>
#include <QVector>

#include <memory>
#include <algorithm>

using namespace Engine;

#ifdef Q_OS_WIN
	void Callbacks::destroy_notify(gpointer data) {}

	GstBusSyncReply
	EngineCallbacks::bus_message_received(GstBus* bus, GstMessage* msg, gpointer data) {
		if(bus_state_changed(bus, msg, data)){
			gst_message_unref(msg);
			return GST_BUS_DROP;
		}

		gst_message_unref(msg);
		return GST_BUS_DROP;
	}
#endif

static bool parse_image(GstTagList* tags, QImage& img)
{
	GstSample* sample;
	bool success = gst_tag_list_get_sample(tags, GST_TAG_IMAGE, &sample);

	if(!success)
	{
		success = gst_tag_list_get_sample(tags, GST_TAG_PREVIEW_IMAGE, &sample);
		if(!success){
			return false;
		}
	}

	GstCaps* caps = gst_sample_get_caps(sample);
	if(!caps){
		gst_sample_unref(sample);
		return false;
	}

	gchar* mimetype = gst_caps_to_string(caps);
	if(mimetype == nullptr){
		gst_sample_unref(sample);
		return false;
	}

	QString mime;
	QString full_mime(mimetype);
	g_free(mimetype); mimetype = nullptr;

	QRegExp re(".*(image/[a-z|A-Z]+).*");
	if(re.indexIn(full_mime) >= 0){
		mime = re.cap(1);
	}

	sp_log(Log::Develop, "Engine Callbacks") << "Cover in Track: " << full_mime;

	GstBuffer* buffer = gst_sample_get_buffer(sample);
	if(!buffer){
		gst_sample_unref(sample);
		return false;
	}

	gsize size = gst_buffer_get_size(buffer);
	if(size == 0){
		gst_sample_unref(sample);
		return false;
	}

	gchar* data = new gchar[size];
	size = gst_buffer_extract(buffer, 0, data, size);

	if(size == 0) {
		delete[] data;
		gst_sample_unref(sample);
		return false;
	}

	img = QImage::fromData((const uchar*) data, size, mime.toLocal8Bit().data());

	delete[] data;
	gst_sample_unref(sample);

	return (!img.isNull());
}


// check messages from bus
gboolean Callbacks::bus_state_changed(GstBus* bus, GstMessage* msg, gpointer data)
{
	Q_UNUSED(bus);

	static MetaData md;
	Base* engine = static_cast<Base*>(data);
	if(!engine){
		return true;
	}

	GstMessageType msg_type = GST_MESSAGE_TYPE(msg);
	QString	msg_src_name = QString(GST_MESSAGE_SRC_NAME(msg)).toLower();
	GstElement*	src = reinterpret_cast<GstElement*>(msg->src);

	switch (msg_type)
	{
		case GST_MESSAGE_EOS:

			if(  !msg_src_name.contains("sr_filesink") &&
				 !msg_src_name.contains("level_sink") &&
				 !msg_src_name.contains("spectrum_sink") &&
				 !msg_src_name.contains("pipeline"))
			{
				sp_log(Log::Debug, "Engine Callbacks") << "EOF reached: " << msg_src_name;
				break;
			}

			engine->set_track_finished(src);

			break;

		case GST_MESSAGE_ELEMENT:
			if(msg_src_name.compare("spectrum") == 0){
				return spectrum_handler(bus, msg, engine);
			}

			if(msg_src_name.compare("level") == 0){
				return level_handler(bus, msg, engine);
			}

			break;

		case GST_MESSAGE_SEGMENT_DONE:
			sp_log(Log::Debug) << "Segment done: " << msg_src_name;
			break;

		case GST_MESSAGE_TAG:
		{
			bool			success;
			Bitrate			bitrate;

			if( msg_src_name.compare("sr_filesink") == 0 ||
				msg_src_name.compare("level_sink") == 0 ||
				msg_src_name.compare("spectrum_sink") == 0)
			{
				break;
			}

			GstTagList*	tags = nullptr;
			gst_message_parse_tag(msg, &tags);
			if(!tags){
				break;
			}

			QImage img;
			success = parse_image(tags, img);
			if(success){
				engine->update_cover(img, src);
			}

			success = gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &bitrate);
			if(success){
				engine->update_bitrate((bitrate / 1000) * 1000, src);
			}

			gchar* title=nullptr;
			success = gst_tag_list_get_string(tags, GST_TAG_TITLE, (gchar**) &title);
			if(success)
			{
				md.set_title(title);
				engine->update_metadata(md, src);

				g_free(title);
			}

			gst_tag_list_unref(tags);
		}

		break;

		case GST_MESSAGE_STATE_CHANGED:
			GstState old_state, new_state, pending_state;

			gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
			/*sp_log(Log::Debug, this) << GST_MESSAGE_SRC_NAME(msg) << ": "
							   << "State changed from "
							   << gst_element_state_get_name(old_state)
							   << " to "
							   << gst_element_state_get_name(new_state)
							   << " pending: "
							   << gst_element_state_get_name(pending_state);*/

			if(!msg_src_name.contains("pipeline", Qt::CaseInsensitive)){
				break;
			}

			if( new_state == GST_STATE_PLAYING ||
				new_state == GST_STATE_PAUSED)
			{
				engine->set_track_ready(src);
			}

			break;

		case GST_MESSAGE_BUFFERING:

			gint percent;

			gint avg_in, avg_out;
			int64_t buffering_left;

			GstBufferingMode mode;
			gst_message_parse_buffering(msg, &percent);
			gst_message_parse_buffering_stats(msg, &mode, &avg_in, &avg_out, &buffering_left );

			engine->set_buffer_state(percent, src);
			break;

		case GST_MESSAGE_DURATION_CHANGED:
			{
				NanoSeconds duration_ns;
				bool success = gst_element_query_duration(src, GST_FORMAT_TIME, &duration_ns);
				if(success) {
					engine->update_duration(GST_TIME_AS_MSECONDS(duration_ns), src);
				}
			}
			break;

		case GST_MESSAGE_INFO:
			/*gst_message_parse_info(msg, &err, nullptr);*/
			break;

		case GST_MESSAGE_WARNING:
			{
				GError*	err;
				gst_message_parse_warning(msg, &err, nullptr);
				sp_log(Log::Warning) << "Engine " << (int) engine->name() << ": GST_MESSAGE_WARNING: " << err->message << ": "
					 << GST_MESSAGE_SRC_NAME(msg);
				g_error_free(err);
			}
			break;

		case GST_MESSAGE_ERROR:
			{
				GError*	err;
				gst_message_parse_error(msg, &err, nullptr);

				sp_log(Log::Error) << "Engine " << (int) engine->name() << ": GST_MESSAGE_ERROR: " << err->message << ": "
						 << GST_MESSAGE_SRC_NAME(msg);

				QString	error_msg(err->message);
				engine->error(error_msg);

				g_error_free(err);
			}
			break;

		case GST_MESSAGE_STREAM_STATUS:
			/*{
				GstStreamStatusType type;
				gst_message_parse_stream_status(msg, &type, NULL);
				sp_log(Log::Debug) << "Get stream status " << type;
			}*/
			break;

		default:
			break;
	}

	return true;
}


// level changed
gboolean
Callbacks::level_handler(GstBus* bus, GstMessage* message, gpointer data)
{
	Q_UNUSED(bus);

	double					channel_values[2];

	Playback* engine = static_cast<Playback*>(data);
	if(!engine) {
		return true;
	}

	const GstStructure* structure = gst_message_get_structure(message);
	if(!structure) {
		sp_log(Log::Warning) << "structure is null";
		return true;
	}

	const gchar* name = gst_structure_get_name(structure);
	if ( strcmp(name, "level") != 0 ) {
		return true;
	}

	const GValue* peak_value = gst_structure_get_value(structure, "peak");
	if(!peak_value) {
		return true;
	}

	GValueArray* rms_arr = static_cast<GValueArray*>(g_value_get_boxed(peak_value));
	guint n_peak_elements = rms_arr->n_values;
	if(n_peak_elements == 0) {
		return true;
	}

	n_peak_elements = std::min((guint) 2, n_peak_elements);
	for(guint i=0; i<n_peak_elements; i++)
	{
		const GValue* val = rms_arr->values + i;

		if(!G_VALUE_HOLDS_DOUBLE(val)) {
			sp_log(Log::Debug) << "Could not find a double";
			break;
		}

		double d = g_value_get_double(val);
		if(d < 0){
			channel_values[i] = d;
		}
	}

	if(n_peak_elements >= 2) {
		engine->set_level(channel_values[0], channel_values[1]);
	}

	else if(n_peak_elements == 1) {
		engine->set_level(channel_values[0], channel_values[0]);
	}

	return true;
}


// spectrum changed
gboolean
Callbacks::spectrum_handler(GstBus* bus, GstMessage* message, gpointer data)
{
	Q_UNUSED(bus);

	static SpectrumList	spectrum_vals;

	Playback* engine = static_cast<Playback*>(data);
	if(!engine) {
		return true;
	}

	const GstStructure* structure = gst_message_get_structure(message);
	if(!structure) {
		return true;
	}

	const gchar* structure_name = gst_structure_get_name(structure);
	if( strcmp(structure_name, "spectrum") != 0 ) {
		return true;
	}

	const GValue* magnitudes = gst_structure_get_value (structure, "magnitude");

	int bins = engine->get_spectrum_bins();
	if(spectrum_vals.empty()){
		spectrum_vals.resize(bins, 0);
	}

	for (int i=0; i<bins; ++i)
	{
		const GValue* mag = gst_value_list_get_value(magnitudes, i);
		if(!mag) {
			continue;
		}

		float f = g_value_get_float(mag);

		spectrum_vals[i] = f;
	}

	engine->set_spectrum(spectrum_vals);

	return true;
}
