/* EqualizerHandler.cpp */

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

#include "Equalizer.h"
#include "Components/Engine/EngineUtils.h"

#include <QList>
#include <QString>

namespace
{
	GstElement* createEqualizerElement()
	{
		GstElement* element {nullptr};
		Engine::Utils::createElement(&element, "equalizer-10bands");

		return element;
	}

	class EqualizerImpl :
		public PipelineExtensions::Equalizer
	{
		public:
			EqualizerImpl() :
				m_element {createEqualizerElement()} {}

			~EqualizerImpl() override = default;

			void setBand(const int bandIndex, const int value) override
			{
				const auto bandName = QString("band%1").arg(bandIndex);
				const auto newValue = (value > 0)
				                      ? (value * 0.25) // NOLINT(readability-magic-numbers)
				                      : (value * 0.75); // NOLINT(readability-magic-numbers)

				Engine::Utils::setValue(m_element, bandName.toUtf8().data(), newValue);
			}

			GstElement* equalizerElement() override { return m_element; }

		private:
			GstElement* m_element;
	};
}

namespace PipelineExtensions
{
	std::shared_ptr<Equalizer> createEqualizer()
	{
		return std::make_shared<EqualizerImpl>();
	}
}
