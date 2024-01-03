/* BroadcastBin.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "Broadcasting.h"
#include "Probing.h"
#include "Components/Engine/EngineUtils.h"
#include "Components/Engine/Callbacks.h"

#include <QString>
#include <QList>

#include <gst/app/gstappsink.h>

namespace
{
	GstPadProbeReturn onDeactivated(GstPad*, GstPadProbeInfo*, gpointer) { return GST_PAD_PROBE_DROP; }

	GstFlowReturn newBuffer(GstElement* sink, gpointer p)
	{
		constexpr const auto TcpBufferSize = 16384U;

		auto data = QByteArray(TcpBufferSize, 0);
		auto* rawDataReceiver = static_cast<PipelineExtensions::RawDataReceiver*>(p);
		if(!rawDataReceiver)
		{
			return GST_FLOW_OK;
		}

		auto* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink)); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
		if(!sample)
		{
			return GST_FLOW_OK;
		}

		auto* buffer = gst_sample_get_buffer(sample);
		if(!buffer)
		{
			gst_sample_unref(sample);
			return GST_FLOW_OK;
		}

		const auto bufferSize = gst_buffer_get_size(buffer);
		const auto newSize = gst_buffer_extract(buffer, 0, data.data(), bufferSize);

		data.resize(static_cast<int>(newSize));
		rawDataReceiver->setRawData(data);

		gst_sample_unref(sample);

		return GST_FLOW_OK;
	}

	class BroadcasterImpl :
		public PipelineExtensions::Broadcaster
	{
		public:
			BroadcasterImpl(PipelineExtensions::RawDataReceiverPtr rawDataReceiver, GstElement* pipeline,
			                GstElement* tee) :
				m_rawDataReceiver(std::move(rawDataReceiver)),
				m_pipeline(pipeline),
				m_tee(tee) {}

			~BroadcasterImpl() override = default;

			bool setEnabled(const bool b) override
			{
				if(b && !init())
				{
					return false;
				}

				if(m_isRunning == b)
				{
					return true;
				}

				m_isRunning = b;
				m_probingData.active = b;
				m_probingData.queue = m_queue;

				PipelineExtensions::Probing::handleProbe(&m_probingData, onDeactivated);

				return true;
			}

			[[nodiscard]] bool isEnabled() const override { return m_isRunning; }

		private:
			bool createElements()
			{
				if(!Engine::Utils::createElement(&m_queue, "queue", "bc_lame_queue") ||
				   !Engine::Utils::createElement(&m_converter, "audioconvert", "bc_lame_converter") ||
				   !Engine::Utils::createElement(&m_resampler, "audioresample", "bc_lame_resampler") ||
				   !Engine::Utils::createElement(&m_lame, "lamemp3enc", "bc_lamemp3enc") ||
				   !Engine::Utils::createElement(&m_appSink, "appsink", "bc_lame_appsink"))
				{
					return false; // NOLINT(readability-simplify-boolean-expr)
				}

				return true;
			}

			bool initBin()
			{
				const auto binCreated =
					Engine::Utils::createBin(&m_bin,
					                         {m_queue, m_converter, m_resampler, m_lame, m_appSink},
					                         "broadcast");
				if(!binCreated)
				{
					return false;
				}

				gst_bin_add(GST_BIN(m_pipeline), m_bin);
				const auto success = Engine::Utils::connectTee(m_tee, m_bin, "BroadcastQueue");
				if(!success)
				{
					Engine::Utils::setState(m_bin, GST_STATE_NULL);
					gst_object_unref(m_bin);
				}

				return success;
			}

			void configureElements()
			{
				gst_object_ref(m_appSink);

				Engine::Utils::configureLame(m_lame);
				Engine::Utils::configureQueue(m_queue);
				Engine::Utils::configureSink(m_appSink);
				Engine::Utils::setValues(G_OBJECT(m_appSink), "emit-signals", true);

				g_signal_connect (m_appSink, "new-sample",
				                  G_CALLBACK(newBuffer), m_rawDataReceiver.get());
			}

			bool init()
			{
				if(m_bin)
				{
					return true;
				}

				if(createElements() && initBin())
				{
					configureElements();
					return true;
				}

				return false;
			}

			PipelineExtensions::RawDataReceiverPtr m_rawDataReceiver;
			PipelineExtensions::Probing::GenericProbingData m_probingData;
			GstElement* m_pipeline;
			GstElement* m_tee {nullptr};

			GstElement* m_bin {nullptr};
			GstElement* m_queue {nullptr};
			GstElement* m_converter {nullptr};
			GstElement* m_resampler {nullptr};
			GstElement* m_lame {nullptr};
			GstElement* m_appSink {nullptr};

			bool m_isRunning {false};
	};

	class RawDataReceiverImpl :
		public PipelineExtensions::RawDataReceiver
	{
		public:
			explicit RawDataReceiverImpl(std::function<void(const QByteArray&)>&& callback) :
				m_callback {std::move(callback)} {}

			~RawDataReceiverImpl() override = default;

			void setRawData(const QByteArray& data) override
			{
				m_callback(data);
			}

		private:
			std::function<void(const QByteArray&)> m_callback;
	};
}

namespace PipelineExtensions
{
	RawDataReceiver::~RawDataReceiver() = default;

	Broadcastable::~Broadcastable() = default;

	Broadcaster::~Broadcaster() = default;

	std::shared_ptr<Broadcaster>
	createBroadcaster(const RawDataReceiverPtr& dataReceiver, GstElement* pipeline, GstElement* tee)
	{
		return std::make_shared<BroadcasterImpl>(dataReceiver, pipeline, tee);
	}

	RawDataReceiverPtr createRawDataReceiver(std::function<void(const QByteArray&)>&& callback)
	{
		return std::make_shared<RawDataReceiverImpl>(std::move(callback));
	}
}