/* PreferenceRegistry.cpp */

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



#include "PreferenceRegistry.h"

#include <QList>
#include <QString>

struct PreferenceRegistry::Private
{
	QList<QString>	preferences;
	PreferenceUi*	user_interface=nullptr;
};

PreferenceRegistry::PreferenceRegistry()
{
	m = Pimpl::make<Private>();
}

PreferenceRegistry::~PreferenceRegistry() {}

void PreferenceRegistry::register_preference(const QString& name)
{
	if(!m->preferences.contains(name))
	{
		m->preferences << name;
	}
}

void PreferenceRegistry::set_user_interface(PreferenceUi* ui)
{
	m->user_interface = ui;
}

void PreferenceRegistry::show_preference(const QString& name)
{
	if(m->user_interface){
		m->user_interface->show_preference(name);
	}
}
