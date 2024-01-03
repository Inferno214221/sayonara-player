/* StreamRecorderHandler.cpp */

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

#include "StreamRecorderData.h"
#include "StreamRecorderBin.h"

#include "Components/Engine/EngineUtils.h"
#include "Components/Engine/PipelineExtensions/Probing.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/StandardPaths.h"

#include <QString>

namespace
{
	class StreamRecorderBinImpl :
		public PipelineExtensions::StreamRecorderBin
	{
		public:
			StreamRecorderBinImpl(GstElement* pipeline, GstElement* tee) :
				m_pipeline {pipeline},
				m_tee {tee} {}

			~StreamRecorderBinImpl() override = default;

			bool init() override
			{
				if(m_bin)
				{
					return true;
				}

				if(createBinElements() && createBin() && connectToTee())
				{
					m_data.queue = m_queue;
					m_data.sink = m_sink;

					return true;
				}

				return false;
			}

			void setTargetPath(const QString& path) override
			{
				if(!m_sink)
				{
					return;
				}

				if(const auto isNewPath = (path != m_path); !isNewPath || m_data.busy)
				{
					return;
				}

				m_path = path;
				m_data.filename = strdup(m_path.toUtf8().data());
				m_data.active = !m_path.isEmpty();

				using namespace PipelineExtensions;
				Probing::handleStreamRecorderProbe(&m_data, Probing::streamRecorderProbed);
			}

		private:
			bool createBinElements()
			{
				if(!Engine::Utils::createElement(&m_queue, "queue", "sr_queue") ||
				   !Engine::Utils::createElement(&m_converter, "audioconvert", "sr_converter") ||
				   !Engine::Utils::createElement(&m_resampler, "audioresample", "sr_resample") ||
				   !Engine::Utils::createElement(&m_lame, "lamemp3enc", "sr_lame") ||
				   !Engine::Utils::createElement(&m_sink, "filesink", "sr_filesink"))
				{
					return false;
				}

				configureBinElements();
				return true;
			}

			void configureBinElements()
			{
				constexpr const auto SinkBufferSize = 8192;

				Engine::Utils::configureLame(m_lame);
				Engine::Utils::configureQueue(m_queue);
				Engine::Utils::configureSink(m_sink);

				// this is just to avoid endless warning messages
				Engine::Utils::setValues(
					G_OBJECT(m_sink),
					"location", Util::tempPath(".stream-recorder.mp3").toLocal8Bit().data());

				Engine::Utils::setUintValue(G_OBJECT(m_sink), "buffer-size", SinkBufferSize);
			}

			bool createBin()
			{
				const auto success = Engine::Utils::createBin(
					&m_bin,
					{m_queue, m_converter, m_resampler, m_lame, m_sink},
					"sr");

				if(success)
				{
					gst_bin_add(GST_BIN(m_pipeline), m_bin);
				}

				return success;
			}

			bool connectToTee()
			{
				const auto success = Engine::Utils::connectTee(m_tee, m_bin, "StreamRecorderQueue");
				if(!success)
				{
					Engine::Utils::setState(m_bin, GST_STATE_NULL);
					gst_object_unref(m_bin);
				}

				return success;
			}

			GstElement* m_pipeline;
			GstElement* m_tee;
			GstElement* m_bin = nullptr;
			GstElement* m_queue = nullptr;
			GstElement* m_converter = nullptr;
			GstElement* m_sink = nullptr;
			GstElement* m_resampler = nullptr;
			GstElement* m_lame = nullptr;

			StreamRecorder::Data m_data;
			QString m_path;
	};
}

namespace PipelineExtensions
{
	StreamRecorderBin* StreamRecorderBin::create(GstElement* pipeline, GstElement* tee)
	{
		return new StreamRecorderBinImpl(pipeline, tee);
	}
}
