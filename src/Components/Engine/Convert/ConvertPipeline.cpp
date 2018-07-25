/* ConvertPipeline.cpp */

/* Copyright 2011-2017  Lucio Carreras
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

#include "ConvertPipeline.h"
#include "Components/Engine/Callbacks/PipelineCallbacks.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

using Pipeline::Convert;
using Pipeline::test_and_error;
using Pipeline::test_and_error_bool;

Convert::Convert(Engine::Base* engine, QObject *parent) :
	Pipeline::Base("ConvertPipeline", engine, parent)
{
	_audio_src = nullptr;
	_lame = nullptr;
	_decoder = nullptr;
	_audio_convert = nullptr;
	_audio_sink = nullptr;
	_resampler = nullptr;
	_xingheader = nullptr;
}


Convert::~Convert() {}

bool Convert::init(GstState state){
	if(!Base::init(state)){
		return false;
	}

	_settings->set<SetNoDB::MP3enc_found>((_lame != nullptr) );
	return true;
}

GstElement*Convert::get_source() const
{
	return _audio_src;
}

bool Convert::create_elements()
{
	if(!create_element(&_audio_src, "uridecodebin", "src")) return false;
	if(!create_element(&_audio_convert, "audioconvert", "audio_convert")) return false;
	if(!create_element(&_lame, "lamemp3enc", "lame")) return false;
	if(!create_element(&_resampler, "audioresample", "resampler")) return false;
	if(!create_element(&_xingheader, "xingmux", "xingmux")) return false;
	if(!create_element(&_audio_sink, "filesink", "filesink")) return false;

	return true;
}

bool Convert::add_and_link_elements()
{
	bool success;

	gst_bin_add_many(GST_BIN(pipeline()),
		_audio_src,
		_audio_convert,
		_resampler,
		_lame,
		_xingheader,
		_audio_sink,
		nullptr
	);

	success = gst_element_link_many(_audio_convert, _resampler, _lame, _xingheader, _audio_sink, nullptr);
	return test_and_error_bool(success, "ConvertEngine: Cannot link lame elements");
}

bool Convert::configure_elements()
{
	g_signal_connect (_audio_src, "pad-added", G_CALLBACK (Callbacks::decodebin_ready), _audio_convert);
	return true;
}


bool Convert::set_uri(gchar* uri)
{
	if(!pipeline()) {
		return false;
	}

	stop();

	g_object_set(G_OBJECT(_audio_src), "uri", uri, nullptr);

	return true;
}

bool Convert::set_target_uri(gchar* uri)
{
	if(!pipeline()) {
		return false;
	}

	stop();
	sp_log(Log::Debug, this) << "Set target uri = " << uri;
	g_object_set(G_OBJECT(_audio_sink), "location", uri, nullptr);
	return true;
}


void Convert::play()
{
	LameBitrate q = (LameBitrate) _settings->get<Set::Engine_ConvertQuality>();
	set_quality(q);

	sp_log(Log::Debug, this) << "Convert pipeline: play";

	Base::play();
}


void Convert::stop()
{
	Base::stop();
}


void Convert::set_quality(LameBitrate quality)
{
	if(!pipeline()) return;

	int cbr=-1;
	double vbr=-1.0;

	switch(quality) {
		case LameBitrate_64:
		case LameBitrate_128:
		case LameBitrate_192:
		case LameBitrate_256:
		case LameBitrate_320:
			cbr = (int) quality;
			break;

		case LameBitrate_var_0:
		case LameBitrate_var_1:
		case LameBitrate_var_2:
		case LameBitrate_var_3:
		case LameBitrate_var_4:
		case LameBitrate_var_5:
		case LameBitrate_var_6:
		case LameBitrate_var_7:
		case LameBitrate_var_8:
		case LameBitrate_var_9:
			vbr = (double) ((int) quality);
			break;

		default:
			cbr = LameBitrate_192;
			break;
	}


	if(cbr > 0) {
		sp_log(Log::Info) << "Set Constant bitrate: " << cbr;
		g_object_set(_lame,
					 "cbr", true,
					 "bitrate", cbr,
					 "target", 1,
					 "encoding-engine-quality", 2,
					 nullptr);
	}

	else {
		sp_log(Log::Info) << "Set variable bitrate: " << vbr;
		g_object_set(_lame,
					 "cbr", false,
					 "quality", vbr,
					 "target", 0,
					 "encoding-engine-quality", 2,
					 nullptr);
	}
}
