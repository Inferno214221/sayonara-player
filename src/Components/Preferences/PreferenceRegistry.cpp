/* PreferenceRegistry.cpp */

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

#include "PreferenceRegistry.h"

#include <QList>
#include <QString>

struct PreferenceRegistry::Private
{
	QList<QString> preferences;
	PreferenceUi* preferenceUserInterface = nullptr;
};

PreferenceRegistry::PreferenceRegistry()
{
	m = Pimpl::make<Private>();
}

PreferenceRegistry::~PreferenceRegistry() {}

void PreferenceRegistry::registerPreference(const QString& name)
{
	if(!m->preferences.contains(name))
	{
		m->preferences << name;
	}
}

void PreferenceRegistry::setUserInterface(PreferenceUi* ui)
{
	m->preferenceUserInterface = ui;
}

void PreferenceRegistry::showPreference(const QString& name)
{
	if(m->preferenceUserInterface)
	{
		m->preferenceUserInterface->showPreference(name);
	}
}
