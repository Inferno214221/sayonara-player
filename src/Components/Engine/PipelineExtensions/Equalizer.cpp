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

#include "Utils/globals.h"
#include "Utils/Settings/Settings.h"
#include "Utils/EqualizerSetting.h"

#include <QList>
#include <QString>

using namespace PipelineExtensions;

struct Equalizer::Private
{
	GstElement* equalizer=nullptr;

	Private()
	{
		Engine::Utils::createElement(&equalizer, "equalizer-10bands");
	}
};

Equalizer::Equalizer()
{
	m = Pimpl::make<Private>();

	int previousIndex = GetSetting(Set::Eq_Last);

	QList<EqualizerSetting> presets = GetSetting(Set::Eq_List);
	presets.push_front(EqualizerSetting());

	if( !Util::between(previousIndex, presets)){
		previousIndex = 0;
	}

	EqualizerSetting last_preset = presets[previousIndex];
	EqualizerSetting::ValueArray values = last_preset.values();

	for(unsigned i=0; i<values.size(); i++)
	{
		setBand(i, values[i]);
	}
}

Equalizer::~Equalizer() {}

void Equalizer::setBand(int band, int val)
{
	QString bandName = QString("band%1").arg(band);

	if(!m->equalizer){
		return;
	}

	double newValue;
	if (val > 0) {
		newValue = val * 0.25;
	}

	else{
		newValue = val * 0.75;
	}

	Engine::Utils::setValue(m->equalizer, bandName.toUtf8().data(),	newValue);
}

GstElement* Equalizer::element() const
{
	return m->equalizer;
}

