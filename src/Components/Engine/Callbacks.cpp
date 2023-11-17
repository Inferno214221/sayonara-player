/* EngineCallbacks.cpp */

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

#include "Callbacks.h"
#include "Components/Engine/EngineUtils.h"
#include "Components/Engine/Engine.h"
#include "Components/Engine/Pipeline.h"

#include "Utils/Utils.h"
#include "Utils/WebAccess/Proxy.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"
#include "Utils/globals.h"

#include <QList>
#include <QRegExp>
#include <QByteArray>

#include <memory>
#include <algorithm>
#include <vector>

#include <gst/gst.h>

namespace EngineUtils = ::Engine::Utils;
namespace Callbacks = ::Engine::Callbacks;
namespace
{
	constexpr const auto DeepLoggingEnabled = false;
	constexpr const auto* ClassEngineCallbacks = "Engine Callbacks";

	QStringList supportedTags {
		GST_TAG_TITLE,
		GST_TAG_ARTIST,
		GST_TAG_ALBUM,
		GST_TAG_ALBUM_ARTIST,
		GST_TAG_COMMENT,

		GST_TAG_PERFORMER,
		GST_TAG_HOMEPAGE,
		GST_TAG_DESCRIPTION,
		GST_TAG_ORGANIZATION,
		GST_TAG_CONTACT,
		GST_TAG_SHOW_NAME,
		GST_TAG_PUBLISHER
	};

	bool isSoupSource(GstElement* source)
	{
		auto* factory = gst_element_get_factory(source);
		const auto elementType = gst_element_factory_get_element_type(factory);
		const auto* name = g_type_name(elementType);

		return (name && QString(name).toLower() == "gstsouphttpsrc");
	}

	GstStructure* getSoundcloudOAuthStructure()
	{
		const auto soundcloudAuthToken = GetSetting(SetNoDB::Soundcloud_AuthToken);
		const auto oauthTokenValue = QString("SoundcloudAuth,Authorization=\"OAuth\\ %1\"")
			.arg(soundcloudAuthToken);

		return gst_structure_new_from_string(oauthTokenValue.toLocal8Bit().data());
	}

	bool hasSoundcloudUri(GstElement* source)
	{
		gchar* uri = nullptr;
		g_object_get(source, "location", &uri, nullptr);
		return (uri && !strncmp(uri, "https://api.soundcloud.com", 26));
	}

	bool parseTags(MetaData& metadata, const GstTagList* tags)
	{
		auto wasUpdated = false;

		for(const auto& tag: supportedTags)
		{
			gchar* value = nullptr;
			auto hasTag = gst_tag_list_get_string(tags, tag.toLocal8Bit().constData(), &value);
			if(!hasTag)
			{
				continue;
			}

			wasUpdated = true;

			if(tag == GST_TAG_TITLE)
			{
				metadata.setTitle(value);
			}

			else if(tag == GST_TAG_ARTIST)
			{
				metadata.setArtist(value);
			}

			else if(tag == GST_TAG_ALBUM)
			{
				metadata.setAlbum(value);
			}

			else if(tag == GST_TAG_ALBUM_ARTIST)
			{
				metadata.setAlbumArtist(value);
			}

			else if(tag == GST_TAG_COMMENT)
			{
				metadata.setComment(value);
			}

			else
			{
				const gchar* nick = gst_tag_get_nick(tag.toLocal8Bit().constData());

				QString sNick = tag;
				if(nick && strnlen(nick, 3) > 0)
				{
					sNick = QString::fromLocal8Bit(nick);
				}

				metadata.replaceCustomField(tag, Util::stringToFirstUpper(sNick), value);
			}

			g_free(value);
		}

		return wasUpdated;
	}

	void updateMetadata(GstTagList* tags, GstElement* srcElement, ::Engine::Engine* engine)
	{
		auto track = engine->currentTrack();
		if(track.isUpdatable())
		{
			const auto wasUpdated = parseTags(track, tags);
			if(wasUpdated)
			{
				engine->updateMetadata(track, srcElement);
			}
		}
	}

	void updateBitrate(GstTagList* tags, GstElement* srcElement, ::Engine::Engine* engine)
	{
		Bitrate bitrate;
		const auto success = gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &bitrate);
		if(success)
		{
			engine->updateBitrate((bitrate / 1000) * 1000, srcElement);
		}
	}

	GstSample* tryFetchImageFromTags(GstTagList* tags)
	{
		GstSample* sample {nullptr};

		if(gst_tag_list_get_sample(tags, GST_TAG_IMAGE, &sample) ||
		   gst_tag_list_get_sample(tags, GST_TAG_PREVIEW_IMAGE, &sample))
		{
			return sample;
		}

		return nullptr;
	}

	QString extractMimeData(GstSample* sample)
	{
		auto* caps = gst_sample_get_caps(sample);
		if(!caps)
		{
			return {};
		}

		auto* capsString = gst_caps_to_string(caps);
		if(!capsString)
		{
			return {};
		}

		const auto mimeData = QString(capsString);
		g_free(capsString);

		auto re = QRegExp(".*(image/[a-z|A-Z]+).*");
		return (re.indexIn(mimeData) >= 0)
		       ? re.cap(1)
		       : QString {};
	}

	QByteArray extractBuffer(GstSample* sample)
	{
		if(auto* buffer = gst_sample_get_buffer(sample); buffer != nullptr)
		{
			if(const auto size = gst_buffer_get_size(buffer); size > 0)
			{
				auto data = QByteArray(static_cast<int>(size), 0x00);
				const auto extractedBytes = gst_buffer_extract(buffer, 0, data.data(), size);
				data.resize(static_cast<int>(extractedBytes));
				return data;
			}
		}

		return {};
	}

	bool updateCoverImage(GstTagList* tags, GstElement* src, ::Engine::Engine* engine)
	{
		auto* sample = tryFetchImageFromTags(tags);
		if(!sample)
		{
			return false;
		}

		const auto mimeType = extractMimeData(sample);
		const auto coverData = extractBuffer(sample);
		const auto isDataValid = !mimeType.isEmpty() && !coverData.isEmpty();
		if(isDataValid)
		{
			spLog(Log::Develop, "Engine Callbacks")
				<< "Cover in Track: " << mimeType
				<< coverData.size() << " bytes.";
			engine->updateCover(src, coverData, mimeType);
		}

		gst_sample_unref(sample);

		return isDataValid;
	}

	void updateCurrentTrack(GstMessage* message, GstElement* srcElement, ::Engine::Engine* engine)
	{
		GstTagList* tags {nullptr};
		gst_message_parse_tag(message, &tags);

		if(tags)
		{
			updateMetadata(tags, srcElement, engine);
			updateBitrate(tags, srcElement, engine);
			updateCoverImage(tags, srcElement, engine);

			gst_tag_list_unref(tags);
		}
	}

	void handleStateChange(GstMessage* message, ::Engine::Engine* engine)
	{
		auto* sourceElement = GST_ELEMENT(message->src); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
		const auto messageSourceName = QString(GST_MESSAGE_SRC_NAME(message));
		auto oldState = GST_STATE_NULL;
		auto newState = GST_STATE_NULL;
		auto pendingState = GST_STATE_NULL;
		gst_message_parse_state_changed(message, &oldState, &newState, &pendingState);

		if constexpr(DeepLoggingEnabled)
		{
			spLog(Log::Debug, ClassEngineCallbacks) << messageSourceName << ": "
			                                        << "State changed from "
			                                        << gst_element_state_get_name(oldState)
			                                        << " to "
			                                        << gst_element_state_get_name(newState)
			                                        << " pending: "
			                                        << gst_element_state_get_name(pendingState);
		}

		if(messageSourceName.contains("pipeline", Qt::CaseInsensitive))
		{
			if((newState == GST_STATE_PLAYING) ||
			   (newState == GST_STATE_PAUSED) ||
			   (newState == GST_STATE_READY))
			{
				engine->setTrackReady(sourceElement);
			}
		}
	}

	int getBufferState(GstMessage* message)
	{
		auto percent = 0;
		auto averageIn = 0;
		auto averageOut = 0;
		auto bufferingLeft = 0L;
		auto bufferingMode = GstBufferingMode::GST_BUFFERING_STREAM;

		gst_message_parse_buffering(message, &percent);
		gst_message_parse_buffering_stats(message, &bufferingMode, &averageIn, &averageOut, &bufferingLeft);

		spLog(Log::Crazy, ClassEngineCallbacks) << "Buffering: " << percent;
		spLog(Log::Crazy, ClassEngineCallbacks) << "Avg In: " << averageIn << " Avg Out: " << averageOut
		                                        << " buffering_left: " << bufferingLeft;

		return static_cast<int>(bufferingLeft);
	}

	QString printMessage(GstMessage* message, const Log logLevel)
	{
		auto messageText = QString {};
		GError* data {nullptr};

		switch(logLevel)
		{
			case Log::Info:
				gst_message_parse_info(message, &data, nullptr);
				break;
			case Log::Warning:
				gst_message_parse_warning(message, &data, nullptr);
				break;
			case Log::Error:
				gst_message_parse_error(message, &data, nullptr);
				break;
			default:
				return {};
		}

		if(data)
		{
			spLog(logLevel, ClassEngineCallbacks) << "Engine: " << data->message << ": "
			                                      << GST_MESSAGE_SRC_NAME(message);
			messageText = QString(data->message);
			g_error_free(data);
		}

		return messageText;
	}

	void printInfo(GstMessage* message) { printMessage(message, Log::Info); }

	void printWarning(GstMessage* message) { printMessage(message, Log::Warning); }

	void handleError(GstMessage* message, ::Engine::Engine* engine)
	{
		static auto errorMessage = QString {};
		const auto newErrorMessage = printMessage(message, Log::Error);

		if(errorMessage != newErrorMessage)
		{
			engine->error(newErrorMessage, QString(GST_MESSAGE_SRC_NAME(message)).toLower());
			errorMessage = newErrorMessage;
		}
	}

	void printStreamStatus([[maybe_unused]] GstMessage* message)
	{
		if constexpr(DeepLoggingEnabled)
		{
			GstStreamStatusType type {GST_STREAM_STATUS_TYPE_CREATE};
			gst_message_parse_stream_status(message, &type, nullptr);
			spLog(Log::Debug, ClassEngineCallbacks) << "Get stream status " << type;
		}
	}
}

// check messages from bus
gboolean Callbacks::busStateChanged([[maybe_unused]] GstBus* bus, GstMessage* message, gpointer data)
{
	auto* engine = static_cast<::Engine::Engine*>(data);
	if(!engine)
	{
		return true;
	}

	const auto messageType = GST_MESSAGE_TYPE(message);
	const auto messageSourceName = QString(GST_MESSAGE_SRC_NAME(message)).toLower();
	auto* sourceElement = GST_ELEMENT(message->src); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

	switch(messageType)
	{
		case GST_MESSAGE_EOS:
			if(!messageSourceName.contains("sr_filesink") &&
			   !messageSourceName.contains("level_sink") &&
			   !messageSourceName.contains("spectrum_sink") &&
			   !messageSourceName.contains("pipeline"))
			{
				spLog(Log::Debug, ClassEngineCallbacks) << "EOF reached: " << messageSourceName;
				break;
			}

			engine->setTrackFinished(sourceElement);
			break;

		case GST_MESSAGE_ELEMENT:
			if(messageSourceName == "spectrum")
			{
				return spectrumHandler(bus, message, engine);
			}

			if(messageSourceName == "level")
			{
				return levelHandler(bus, message, engine);
			}

			break;

		case GST_MESSAGE_SEGMENT_DONE:
			spLog(Log::Debug, ClassEngineCallbacks) << "Segment done: " << messageSourceName;
			break;

		case GST_MESSAGE_TAG:
			if(messageSourceName.contains("fake") ||
			   messageSourceName.contains("lame") ||
			   !messageSourceName.contains("sink"))
			{
				break;
			}

			updateCurrentTrack(message, sourceElement, engine);

			break;

		case GST_MESSAGE_STATE_CHANGED:
			handleStateChange(message, engine);
			break;

		case GST_MESSAGE_BUFFERING:
			engine->setBufferState(getBufferState(message), sourceElement);
			break;

		case GST_MESSAGE_DURATION_CHANGED:
			engine->updateDuration(sourceElement);
			break;

		case GST_MESSAGE_INFO:
			printInfo(message);
			break;

		case GST_MESSAGE_WARNING:
			printWarning(message);
			break;

		case GST_MESSAGE_ERROR:
			handleError(message, engine);
			break;

		case GST_MESSAGE_STREAM_STATUS:
			printStreamStatus(message);
			break;

		default:
			break;
	}

	return true;
}

gboolean Callbacks::levelHandler([[maybe_unused]] GstBus* bus, GstMessage* message, gpointer data)
{
	auto* engine = static_cast<::Engine::Engine*>(data);
	if(!engine)
	{
		return true;
	}

	const auto* structure = gst_message_get_structure(message);
	if(!structure)
	{
		spLog(Log::Warning, ClassEngineCallbacks) << "structure is null";
		return true;
	}

	const auto* name = gst_structure_get_name(structure);
	if(strcmp(name, "level") != 0)
	{
		return true;
	}

	const auto* peakValue = gst_structure_get_value(structure, "peak");
	if(!peakValue)
	{
		return true;
	}

	auto* rmsArray = static_cast<GValueArray*>(g_value_get_boxed(peakValue));
	auto numPeakElements = rmsArray->n_values;
	if(numPeakElements == 0)
	{
		return true;
	}

	auto channelValues = std::array<float, 2> {0.0F};
	numPeakElements = std::min(2U, numPeakElements);

	std::transform(rmsArray->values,
	               rmsArray->values + numPeakElements, // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	               std::begin(channelValues),
	               [](const auto& value) -> float {
		               if(G_VALUE_HOLDS_DOUBLE(&value))
		               {
			               if(const auto d = g_value_get_double(&value); d < 0)
			               {
				               return static_cast<float>(d);
			               }
		               }

		               spLog(Log::Debug, ClassEngineCallbacks) << "Could not find a negative double value";
		               return 0.0F;
	               });

	engine->setLevel(channelValues[0],
	                 (numPeakElements >= 2) ? channelValues[1] : channelValues[0]);

	return true;
}

// spectrum changed
gboolean
Callbacks::spectrumHandler([[maybe_unused]] GstBus* bus, GstMessage* message, gpointer data)
{
	auto* engine = static_cast<::Engine::Engine*>(data);
	if(!engine)
	{
		return true;
	}

	const auto* structure = gst_message_get_structure(message);
	if(!structure)
	{
		return true;
	}

	const auto structureName = QString(gst_structure_get_name(structure));
	if(structureName != "spectrum")
	{
		return true;
	}

	const auto* magnitudes = gst_structure_get_value(structure, "magnitude");
	const auto bins = gst_value_list_get_size(magnitudes);

	static std::vector<float> spectrumValues;
	spectrumValues.resize(bins, 0);

	for(auto i = 0U; i < bins; ++i)
	{
		const auto* magnitude = gst_value_list_get_value(magnitudes, i);
		if(magnitude)
		{
			spectrumValues[i] = g_value_get_float(magnitude);
		}
	}

	engine->setSpectrum(spectrumValues);

	return true;
}

gboolean Callbacks::positionChanged(gpointer data)
{
	auto* pipeline = static_cast<Pipeline*>(data);
	if(!pipeline)
	{
		return false;
	}

	GstState state = pipeline->state();
	if(state != GST_STATE_PLAYING &&
	   state != GST_STATE_PAUSED &&
	   state != GST_STATE_READY)
	{
		return true;
	}

	pipeline->checkPosition();

	return true;
}

// dynamic linking, important for decodebin
void Callbacks::decodebinReady(GstElement* source, GstPad* new_src_pad, gpointer data)
{
	EngineUtils::GStringAutoFree element_name(gst_element_get_name(source));
	spLog(Log::Develop, "Callback") << "Source: " << element_name.data();

	auto* element = static_cast<GstElement*>(data);
	GstPad* sink_pad = gst_element_get_static_pad(element, "sink");
	if(!sink_pad)
	{
		return;
	}

	if(gst_pad_is_linked(sink_pad))
	{
		gst_object_unref(sink_pad);
		return;
	}

	GstPadLinkReturn pad_link_return = gst_pad_link(new_src_pad, sink_pad);

	if(pad_link_return != GST_PAD_LINK_OK)
	{
		spLog(Log::Error, ClassEngineCallbacks) << "Dynamic pad linking: Cannot link pads";

		switch(pad_link_return)
		{
			case GST_PAD_LINK_WRONG_HIERARCHY:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Wrong hierarchy";
				break;
			case GST_PAD_LINK_WAS_LINKED:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Pad was already linked";
				break;
			case GST_PAD_LINK_WRONG_DIRECTION:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Pads have wrong direction";
				break;
			case GST_PAD_LINK_NOFORMAT:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Pads have incompatible format";
				break;
			case GST_PAD_LINK_NOSCHED:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Pads cannot cooperate scheduling";
				break;
			case GST_PAD_LINK_REFUSED:
			default:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Refused because of different reason";
				break;
		}
	}

	else
	{
		spLog(Log::Develop, "Callbacks") << "Successfully linked " << gst_element_get_name(source) << " with "
		                                 << gst_element_get_name(element);
	}

	gst_object_unref(sink_pad);
}

#define TCP_BUFFER_SIZE 16384

GstFlowReturn Callbacks::newBuffer(GstElement* sink, gpointer p)
{
	static char data[TCP_BUFFER_SIZE];

	auto* pipeline = static_cast<PipelineExtensions::BroadcastDataReceiver*>(p);
	if(!pipeline)
	{
		return GST_FLOW_OK;
	}

	GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
	if(!sample)
	{
		return GST_FLOW_OK;
	}

	GstBuffer* buffer = gst_sample_get_buffer(sample);
	if(!buffer)
	{
		gst_sample_unref(sample);
		return GST_FLOW_OK;
	}

	gsize size = gst_buffer_get_size(buffer);
	gsize newSize = gst_buffer_extract(buffer, 0, data, size);

	QByteArray bytes(data, int(newSize));
	pipeline->setRawData(bytes);

	gst_sample_unref(sample);

	return GST_FLOW_OK;
}

void Callbacks::sourceReady(GstURIDecodeBin* /* bin */, GstElement* source, gpointer /* data */)
{
	spLog(Log::Develop, "Engine Callback") << "Source ready: is soup? " << isSoupSource(source);
	gst_base_src_set_dynamic_size(GST_BASE_SRC(source), false);

	if(isSoupSource(source))
	{
		if(Proxy::active())
		{
			spLog(Log::Develop, "Engine Callback") << "Will use proxy: " << Proxy::fullUrl();

			if(Proxy::hasUsername())
			{
				spLog(Log::Develop, "Engine Callback") << "Will use proxy username: " << Proxy::username();

				EngineUtils::setValues(
					source,
					"proxy-id", Proxy::username().toLocal8Bit().data(),
					"proxy-pw", Proxy::password().toLocal8Bit().data());
			}
		}

		if(hasSoundcloudUri(source))
		{
			EngineUtils::setValues(
				source,
				"extra-headers", getSoundcloudOAuthStructure());
		}
	}
}
