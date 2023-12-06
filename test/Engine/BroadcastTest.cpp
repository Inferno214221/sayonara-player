/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "Common/SayonaraTest.h"
#include "Components/Engine/EngineUtils.h"
#include "Components/Engine/PipelineExtensions/BroadcastBin.h"
#include <gstreamer-1.0/gst/audio/audio.h>
#include <gstreamer-1.0/gst/gstbuffer.h>

#include <thread>
#include <future>

// https://gstreamer.freedesktop.org/documentation/tutorials/basic/short-cutting-the-pipeline.html?gi-language=c

namespace
{
	constexpr const auto SampleRate = 44'100;
	constexpr const auto ChunkSize = 1024;
	constexpr const auto Channels = 1;

	struct CustomData
	{
		GstElement* appsrc;
		guint64 numSamples {0};
		guint sourceid {0};
	};

	gboolean pushData(CustomData* data)
	{
		const auto numSamples = ChunkSize / 2; /* Because each sample is 16 bits */

		auto* buffer = gst_buffer_new_and_alloc (ChunkSize);
			GST_BUFFER_TIMESTAMP (buffer) = gst_util_uint64_scale(data->numSamples, GST_SECOND, SampleRate);
		GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale(numSamples, GST_SECOND, SampleRate);

		data->numSamples += numSamples;

		auto ret = GstFlowReturn {GST_FLOW_ERROR};
		g_signal_emit_by_name(data->appsrc, "push-buffer", buffer, &ret);
		gst_buffer_unref(buffer);

		return (ret == GST_FLOW_OK);
	}

	void feedData(GstElement*, guint, CustomData* data)
	{
		if(data->sourceid == 0)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			data->sourceid = g_idle_add(reinterpret_cast<GSourceFunc>(pushData), data);
		}
	}

	void stopFeed(GstElement*, CustomData* data)
	{
		if(data->sourceid != 0)
		{
			g_source_remove(data->sourceid);
			data->sourceid = 0;
		}
	}

	void setStandardCaps(GstElement* element)
	{
		GstAudioInfo info;
		gst_audio_info_set_format(&info, GST_AUDIO_FORMAT_S16, SampleRate, Channels, nullptr);
		auto* caps = gst_audio_info_to_caps(&info);
		Engine::Utils::setValues(element, "caps", caps, "format", GST_FORMAT_TIME);
		gst_caps_unref(caps);
	}

	class BroadcastPipeline
	{
		public:
			BroadcastPipeline() :
				m_customData {m_appsrc, 0, 0}
			{
				Engine::Utils::addElements(GST_BIN(m_pipeline),
				                           {m_appsrc, m_convert, m_resample, m_tee, m_queue, m_fakesink});
				Engine::Utils::linkElements({m_appsrc, m_convert, m_resample, m_tee});
				Engine::Utils::linkElements({m_queue, m_fakesink});

				auto* teeSrcPadTemplate = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(m_tee), "src_%u");
				QVERIFY(teeSrcPadTemplate != nullptr);
				auto* teeQueuePad = gst_element_request_pad(m_tee, teeSrcPadTemplate, nullptr, nullptr);
				QVERIFY(teeQueuePad != nullptr);
				auto* queuePad = gst_element_get_static_pad(m_queue, "sink");
				QVERIFY(queuePad != nullptr);

				auto isTeeLinked = gst_pad_link(teeQueuePad, queuePad);
				QVERIFY(isTeeLinked == GST_PAD_LINK_OK);

				setStandardCaps(m_appsrc);

				g_signal_connect (m_appsrc, "need-data", reinterpret_cast<GCallback>(feedData), &m_customData);
				g_signal_connect (m_appsrc, "enough-data", reinterpret_cast<GCallback>(stopFeed), &m_customData);
			}

			[[nodiscard]] GstElement* pipelineElement() { return m_pipeline; }

			[[nodiscard]] GstElement* teeElement() { return m_tee; }

			void start() { Engine::Utils::setState(m_pipeline, GST_STATE_PLAYING); }

			void stop() { Engine::Utils::setState(m_pipeline, GST_STATE_NULL); }

		private:
			GstElement* m_pipeline {gst_pipeline_new("pipeline")};
			GstElement* m_appsrc {gst_element_factory_make("appsrc", nullptr)};
			GstElement* m_convert {gst_element_factory_make("audioconvert", "audio_convert1")};
			GstElement* m_resample {gst_element_factory_make("audioresample", "audio_resample")};
			GstElement* m_tee {gst_element_factory_make("tee", nullptr)};
			GstElement* m_queue {gst_element_factory_make("queue", nullptr)};
			GstElement* m_fakesink {gst_element_factory_make("fakesink", nullptr)};
			CustomData m_customData;
	};
}

class BroadcastTest :
	public Test::Base
{
	Q_OBJECT

	public:
		BroadcastTest() :
			Test::Base("BroadcastTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
		[[maybe_unused]] void newBufferTest()
		{
			gst_init(nullptr, nullptr);

			auto pipeline = BroadcastPipeline();

			std::promise<QByteArray> promise;
			auto future = promise.get_future();

			auto* loop = g_main_loop_new(nullptr, false);
			auto rawDataReceiver = PipelineExtensions::createRawDataReceiver([&](const auto& bufferData) {
				promise.set_value(bufferData);
				g_main_loop_quit(loop);
			});

			auto broadcaster = PipelineExtensions::createBroadcaster(rawDataReceiver,
			                                                         pipeline.pipelineElement(),
			                                                         pipeline.teeElement());
			broadcaster->setEnabled(true);
			QVERIFY(broadcaster->isEnabled());

			auto thread = std::thread([&]() {
				const auto waitResult = future.wait_for(std::chrono::milliseconds {1000});
				if(waitResult == std::future_status::timeout)
				{
					g_main_loop_quit(loop);
				}
			});

			pipeline.start();

			g_main_loop_run(loop);

			thread.join();

			pipeline.stop();

			QVERIFY(future.get().size() == 417);
		}
};

QTEST_MAIN(BroadcastTest)

#include "BroadcastTest.moc"
