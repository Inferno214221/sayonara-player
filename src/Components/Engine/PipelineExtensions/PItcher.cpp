/* SpeedHandler.cpp */

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

#include "Pitcher.h"

#include "Components/Engine/EngineUtils.h"
#include "Utils/Settings/Settings.h"

namespace
{
	class PitcherImpl :
		public PipelineExtensions::Pitcher
	{
		public:
			PitcherImpl()
			{
				Engine::Utils::createElement(&m_pitch, "pitch");
			}

			~PitcherImpl() override = default;

			[[nodiscard]] GstElement* pitchElement() const override { return m_pitch; }

			void setSpeed(const float speed, const double pitch, const bool preservePitch) override
			{
				if(!GetSetting(Set::Engine_SpeedActive) || !m_pitch)
				{
					return;
				}

				if(preservePitch)
				{
					Engine::Utils::setValues(m_pitch,
					                         "tempo", speed,
					                         "rate", 1.0,
					                         "pitch", pitch);
				}

				else
				{
					Engine::Utils::setValues(m_pitch,
					                         "tempo", 1.0,
					                         "rate", speed,
					                         "pitch", pitch);
				}
			}

		private:
			GstElement* m_pitch {nullptr};
	};
}

namespace PipelineExtensions
{
	Pitcher::~Pitcher() = default;

	std::shared_ptr<Pitcher> createPitcher()
	{
		return std::make_shared<PitcherImpl>();
	}
}