/* Visualizer.cpp */

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

#include "VisualizerBin.h"
#include "Probing.h"
#include "Components/Engine/EngineUtils.h"
#include "Utils/Settings/Settings.h"

namespace
{
	GstPadProbeReturn onDeactivated(GstPad*, GstPadProbeInfo*, gpointer) { return GST_PAD_PROBE_DROP; }
}

class VisualizerBinImpl :
	public PipelineExtensions::VisualizerBin
{
	public:
		VisualizerBinImpl(GstElement* pipeline, GstElement* tee) :
			m_pipeline(pipeline),
			m_tee(tee) {}

		~VisualizerBinImpl() override = default;

		bool setEnabled(const bool levelEnabled, const bool spectrumEnabled) override
		{
			if(!init())
			{
				return false;
			}

			m_isLevelEnabled = levelEnabled;
			m_isSpectrumEnabled = spectrumEnabled;

			const auto isRunning = (levelEnabled || spectrumEnabled);
			if(isRunning == m_isRunning)
			{
				Engine::Utils::setValue(G_OBJECT(m_level), "post-messages", levelEnabled);
				Engine::Utils::setValue(G_OBJECT(m_spectrum), "post-messages", spectrumEnabled);

				return true;
			}

			m_isRunning = isRunning;
			m_probingData.active = isRunning;
			m_probingData.queue = m_queue;

			PipelineExtensions::Probing::handleProbe(&m_probingData, onDeactivated);
			Engine::Utils::setValue(G_OBJECT(m_level), "post-messages", levelEnabled);
			Engine::Utils::setValue(G_OBJECT(m_spectrum), "post-messages", spectrumEnabled);

			return true;
		}

		[[nodiscard]] bool isLevelEnabled() const override { return m_isLevelEnabled; }

		[[nodiscard]] bool isSpectrumEnabled() const override { return m_isSpectrumEnabled; }

	private:
		bool createElements()
		{
			if(Engine::Utils::createElement(&m_queue, "queue", "visualizer") &&
			   Engine::Utils::createElement(&m_level, "level") &&
			   // in case of renaming, also look in EngineCallbase GST_MESSAGE_EVENT
			   Engine::Utils::createElement(&m_spectrum, "spectrum") &&
			   Engine::Utils::createElement(&m_sink, "fakesink", "visualizer"))
			{
				Engine::Utils::createBin(&m_bin, {m_queue, m_level, m_spectrum, m_sink}, "visualizer");
			}

			return (m_bin != nullptr);
		}

		bool linkElements()
		{
			gst_bin_add(GST_BIN(m_pipeline), m_bin);
			const auto success = Engine::Utils::connectTee(m_tee, m_bin, "Visualizer");
			if(!success)
			{
				gst_bin_remove(GST_BIN(m_pipeline), m_bin);
				gst_object_unref(m_bin);
				m_bin = nullptr;
			}

			return success;
		}

		void configureElements()
		{
			Engine::Utils::setValues(G_OBJECT(m_level), "post-messages", true);
			Engine::Utils::setUint64Value(G_OBJECT(m_level), "interval", 20 * GST_MSECOND);
			Engine::Utils::setValues(G_OBJECT (m_spectrum),
			                         "post-messages", true,
			                         "message-phase", false,
			                         "message-magnitude", true,
			                         "multi-channel", false);

			Engine::Utils::setIntValue(G_OBJECT(m_spectrum),
			                           "threshold",
			                           -75); // NOLINT(readability-magic-numbers)
			Engine::Utils::setUintValue(G_OBJECT(m_spectrum), "bands", GetSetting(Set::Engine_SpectrumBins));
			Engine::Utils::setUint64Value(G_OBJECT(m_spectrum),
			                              "interval",
			                              20 * GST_MSECOND); // NOLINT(readability-magic-numbers)

			Engine::Utils::configureQueue(m_queue, 1000); // NOLINT(readability-magic-numbers)
			Engine::Utils::configureSink(m_sink);
		}

		bool init()
		{
			if(m_bin || (createElements() && linkElements()))
			{
				configureElements();
				return true;
			}

			return false;
		}

		GstElement* m_pipeline {nullptr};
		GstElement* m_tee {nullptr};

		GstElement* m_bin {nullptr};
		GstElement* m_queue {nullptr};
		GstElement* m_spectrum {nullptr};
		GstElement* m_level {nullptr};
		GstElement* m_sink {nullptr};

		PipelineExtensions::Probing::GenericProbingData m_probingData;
		bool m_isRunning {false};
		bool m_isSpectrumEnabled {false};
		bool m_isLevelEnabled {false};
};

namespace PipelineExtensions
{
	VisualizerBin::~VisualizerBin() = default;

	std::shared_ptr<VisualizerBin> createVisualizerBin(GstElement* pipeline, GstElement* tee)
	{
		return std::make_shared<VisualizerBinImpl>(pipeline, tee);
	}
}
