/* CrossFader.cpp */

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

/* CrossFader.cpp */

#include "Crossfader.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/typedefs.h"

#include <QTimer>

#include <cmath>
#include <thread>

namespace PipelineExtensions
{
	namespace
	{
		constexpr const auto FadingStepTime = MilliSeconds {20};

		class CrossfaderImpl :
			public PipelineExtensions::Crossfader
		{
			public:
				CrossfaderImpl(PlaystateController* playstateController, VolumeController* volumeController) :
					m_playstateController {playstateController},
					m_volumeController {volumeController} {}

				void fadeIn() override
				{
					if(initFader(Crossfader::FadeMode::FadeIn))
					{
						m_playstateController->play();
					}
				}

				void fadeOut() override
				{
					initFader(Crossfader::FadeMode::FadeOut);
				}

				void abortFading() override
				{
					stopTimer();
				}

			private:
				bool initFader(const Crossfader::FadeMode mode)
				{
					m_mode = mode;
					stopTimer();

					const auto engineVolume = GetSetting(Set::Engine_Vol) / 100.0;
					const auto fadingTime = GetSetting(Set::Engine_CrossFaderTime);

					m_startVolume = (m_mode == Crossfader::FadeMode::FadeIn) ? 0.0 : engineVolume;
					m_targetVolume = (m_mode == Crossfader::FadeMode::FadeOut) ? 0.0 : engineVolume;

					m_volumeController->setVolume(m_startVolume);

					const auto volumeChangePerMillisecond = (m_targetVolume - m_startVolume) / (fadingTime * 1.0);
					const auto volumeChangePerTimerIteration = FadingStepTime * volumeChangePerMillisecond;
					const auto iterations = fadingTime / FadingStepTime;

					spLog(Log::Develop, this) << "Volume change per iteration: " << volumeChangePerTimerIteration
					                          << " every " << FadingStepTime << "ms"
					                          << " from Volume " << m_startVolume << " to " << m_targetVolume;

					constexpr const auto VolumeTolerance = 0.00001;
					const auto isFadeNecessary = (std::fabs(volumeChangePerTimerIteration) > VolumeTolerance);
					if(isFadeNecessary)
					{
						startTimer(iterations, volumeChangePerTimerIteration);
					}

					return isFadeNecessary;
				}

				void startTimer(const int iterations, const double volumeChangePerTimerIteration)
				{
					auto callback = [this, volumeChangePerTimerIteration]() {
						timerStepTriggered(volumeChangePerTimerIteration);
					};

					m_fadingInProgress = true;
					m_timer = std::thread([this, iterations, callback = std::move(callback)]() {
						for(auto i = 0; (i < iterations) && fadingInProgress(); i++)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds {FadingStepTime});
							callback();
						}
					});
				}

				void stopTimer()
				{
					if(m_fadingInProgress)
					{
						m_fadingInProgress = false;
						m_timer.join();
					}
				}

				[[nodiscard]] bool fadingInProgress() const { return m_fadingInProgress; }

				void timerStepTriggered(const double deltaVolume)
				{
					const auto currentVolume = m_volumeController->volume();
					const auto newVolume = currentVolume + deltaVolume;
					auto fadeAllowed = (newVolume <= 1.0) && (newVolume >= 0);

					if(m_mode == Crossfader::FadeMode::FadeIn)
					{
						fadeAllowed &= (newVolume < m_targetVolume);
					}

					else if(m_mode == Crossfader::FadeMode::FadeOut)
					{
						fadeAllowed &= (newVolume > m_targetVolume);
						if(!fadeAllowed)
						{
							m_playstateController->stop();
						}
					}

					if(fadeAllowed)
					{
						if(m_mode == Crossfader::FadeMode::FadeIn)
						{
							spLog(Log::Crazy, this) << "Set volume from " << currentVolume << " to " << newVolume;
						}

						m_volumeController->setVolume(newVolume);
					}

					else
					{
						m_volumeController->setVolume(m_targetVolume);

						stopTimer();
					}
				}

				std::thread m_timer;

				PipelineExtensions::PlaystateController* m_playstateController;
				PipelineExtensions::VolumeController* m_volumeController;

				double m_startVolume {0};
				double m_targetVolume {0};
				bool m_fadingInProgress {false};

				Crossfader::FadeMode m_mode {Crossfader::FadeMode::NoFading};
		};
	}

	MilliSeconds Crossfader::fadingTimeMs()
	{
		return GetSetting(Set::Engine_CrossFaderActive)
		       ? GetSetting(Set::Engine_CrossFaderTime)
		       : 0;
	}

	Crossfader::~Crossfader() = default;

	std::shared_ptr<Crossfader>
	createCrossfader(PlaystateController* playstateController, VolumeController* volumeController)
	{
		return std::make_shared<CrossfaderImpl>(playstateController, volumeController);
	}
}