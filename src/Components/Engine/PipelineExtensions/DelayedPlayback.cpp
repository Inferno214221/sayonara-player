/* DelayedPlayHandler.cpp */

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

#include "DelayedPlayback.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"

#include <gst/gst.h>
#include <gst/base/gstbasesink.h>

#include <thread>
#include <chrono>

namespace
{
	class DelayedPlaybackInvokerImpl :
		public PipelineExtensions::DelayedPlaybackInvoker
	{
		public:
			explicit DelayedPlaybackInvokerImpl(PipelineExtensions::PlaystateController* playstateController) :
				m_playstateController {playstateController} {}

			~DelayedPlaybackInvokerImpl() override
			{
				if(m_timer.joinable())
				{
					m_timer.join();
				}
			}

			void playIn(const MilliSeconds ms) override
			{
				if(m_isRunning)
				{
					return;
				}

				m_isRunning = true;
				m_playstateController->pause();
				if(m_timer.joinable())
				{
					m_timer.join();
				}

				auto callback = [this]() {
					timedOut();
				};

				m_timer = std::thread([this, ms, callback = std::move(callback)]() {
					spLog(Log::Info, this) << "Will start playback in " << ms << "ms";
					std::this_thread::sleep_for(std::chrono::milliseconds {ms} / 2);

					callback();
				});
			}

			void timedOut()
			{
				if(!m_isAborted)
				{
					m_playstateController->play();
				}

				m_isRunning = false;
				m_isAborted = false;
			}

			void abortDelayedPlaying() override
			{
				if(m_isRunning)
				{
					m_isAborted = true;
				}
			}

		private:
			PipelineExtensions::PlaystateController* m_playstateController;
			std::thread m_timer;
			std::atomic<bool> m_isRunning {false};
			bool m_isAborted {false};
	};
}

namespace PipelineExtensions
{
	DelayedPlaybackInvoker::~DelayedPlaybackInvoker() = default;

	std::shared_ptr<DelayedPlaybackInvoker>
	createDelayedPlaybackInvoker(PlaystateController* pipeline)
	{
		return std::make_shared<DelayedPlaybackInvokerImpl>(pipeline);
	}
}