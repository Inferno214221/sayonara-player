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

#include "EqualizerAccesible.h"
#include "Components/Engine/EngineUtils.h"
#include "Components/Equalizer/Equalizer.h"

#include "Utils/globals.h"
#include "Utils/Settings/Settings.h"
#include "Utils/EqualizerSetting.h"

#include <QList>
#include <QString>

using namespace PipelineExtensions;

EqualizerAccessible::EqualizerAccessible() = default;
EqualizerAccessible::~EqualizerAccessible() = default;

void EqualizerAccessible::initEqualizer()
{
	Equalizer equalizer;
	EqualizerSetting lastPreset = equalizer.currentSetting();

	int band=0;
	for(const auto value : lastPreset)
	{
		setEqualizerBand(band++, value);
	}
}

void EqualizerAccessible::setEqualizerBand(int bandIndex, int value)
{
	auto bandName = QString("band%1").arg(bandIndex);

	GstElement* element = this->equalizerElement();
	if(!element){
		return;
	}

	double newValue = (value > 0)
		? (value * 0.25)
		: (value * 0.75);

	Engine::Utils::setValue(element, bandName.toUtf8().data(), newValue);
}

