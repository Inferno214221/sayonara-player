/* EqualizerHandler.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "EqualizerHandler.h"
#include "../Callbacks/EngineUtils.h"

#include "Utils/Settings/Settings.h"
#include "Utils/EqualizerPresets.h"
#include "Utils/globals.h"

#include <QList>
#include <QString>
#include <gst/gst.h>

using Pipeline::Equalizer;

struct Equalizer::Private
{
	GstElement* equalizer=nullptr;

	Private()
	{
		Engine::Utils::create_element(&equalizer, "equalizer-10bands");
	}
};

Equalizer::Equalizer()
{
	m = Pimpl::make<Private>();

	int last_idx = GetSetting(Set::Eq_Last);

	QList<EQ_Setting> presets = GetSetting(Set::Eq_List);
	presets.push_front(EQ_Setting());

	if( !between(last_idx, presets)){
		last_idx = 0;
	}

	EQ_Setting last_preset = presets[last_idx];
	QList<int> values = last_preset.values();

	for(int i=0; i<values.size(); i++)
	{
		set_band(i, values[i]);
	}
}

Equalizer::~Equalizer() {}

void Equalizer::set_band(int band, int val)
{
	QString band_name = QString("band%1").arg(band);

	if(!m->equalizer){
		return;
	}

	double new_val;
	if (val > 0) {
		new_val = val * 0.25;
	}

	else{
		new_val = val * 0.75;
	}

	Engine::Utils::set_value(m->equalizer, band_name.toUtf8().data(),	new_val);
}

GstElement* Equalizer::element() const
{
	return m->equalizer;
}

